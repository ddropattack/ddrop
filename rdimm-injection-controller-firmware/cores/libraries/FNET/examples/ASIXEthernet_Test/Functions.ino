#define HACKED //Limits number of times usbthread loops per second

void usbthread() {
  uint32_t cc = 0;
  while(1) {
    myusb.Task();
    asix1.read();
    checkLink();
    if(fnet_netif_is_initialized(current_netif)){
      fnet_poll();
      fnet_service_poll();
    }
    #ifdef STATS
    LoopedUSB++;
    #endif
    #ifdef HACKED
    cc++;
    if ( cc > 20 ) {
      cc=0;
      threads.yield();
    }
    #endif
  }
}

void dhcp_cln_callback_updated(fnet_dhcp_cln_desc_t _dhcp_desc, fnet_netif_desc_t netif, void *p ) { //Called when DHCP updates
  struct fnet_dhcp_cln_options current_options;
  fnet_dhcp_cln_get_options(dhcp_desc, &current_options);
  
  uint8_t *ip = (uint8_t*)&current_options.ip_address.s_addr;
  Serial.print("IPAddress: ");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.println((uint8_t)*ip);

  ip = (uint8_t*)&current_options.netmask.s_addr;
  Serial.print("SubnetMask: ");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.println((uint8_t)*ip);

  ip = (uint8_t*)&current_options.gateway.s_addr;
  Serial.print("Gateway: ");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.println((uint8_t)*ip);

  ip = (uint8_t*)&current_options.dhcp_server.s_addr;
  Serial.print("DHCPServer: ");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.print((uint8_t)*ip++);
  Serial.print(".");
  Serial.println((uint8_t)*ip);

  
  Serial.print("State: ");
  Serial.println(fnet_dhcp_cln_get_state(_dhcp_desc));
  Serial.println();
  Serial.println();

  

}

void checkLink(){
//  Serial.print("Link: ");
//  Serial.println(asix1.connected);
  if(asix1.connected && fnet_dhcp_cln_is_enabled(dhcp_desc)){
//    Serial.println("DHCP already running!");
  }
  else if(asix1.connected){
    struct fnet_init_params     init_params;
  
    static const fnet_timer_api_t timer_api = { //Setup multi-thread timer
      .timer_get_ms = timer_get_ms,
      .timer_delay = 0,
    };
    /* Input parameters for FNET stack initialization */
    init_params.netheap_ptr = stack_heap;
    init_params.netheap_size = sizeof(stack_heap);
    init_params.mutex_api = &teensy_mutex_api;
    init_params.timer_api = &timer_api;
    /* FNET Initialization */
    if (fnet_init(&init_params) != FNET_ERR) {
      Serial.println("TCP/IP stack initialization is done.\n");
      /* You may use FNET stack API */
      /* Initialize networking interfaces using fnet_netif_init(). */
//        Get current net interface.
      if(fnet_netif_init(FNET_CPU_USB0_IF, MacAddress, 6) != FNET_ERR){
        Serial.println("netif Initialized");
        if((current_netif = fnet_netif_get_default()) == 0){
          Serial.println("ERROR: Network Interface is not configurated!");
        }
        else {
          Serial.println("Initializing DHCP");
          fnet_memset_zero(&dhcp_desc, sizeof(dhcp_desc));
          fnet_memset_zero(&dhcp_params, sizeof(dhcp_params));
          dhcp_params.netif = current_netif;
          // Enable DHCP client.
          if((dhcp_desc = fnet_dhcp_cln_init(&dhcp_params))){
              /*Register DHCP event handler callbacks.*/
             fnet_dhcp_cln_set_callback_updated(dhcp_desc, dhcp_cln_callback_updated, (void*)dhcp_desc);
             fnet_dhcp_cln_set_callback_discover(dhcp_desc, dhcp_cln_callback_updated, (void*)dhcp_desc);
             Serial.println("DHCP initialization done!");
             bench_srv_init();
          }
          else{
            Serial.println("ERROR: DHCP initialization failed!");
          }
        }
      }
      else {
        Serial.println("Error:netif initialization failed.\n");
      }
  }
    else {
      Serial.println("Error:TCP/IP stack initialization failed.\n");
    }
  }
  else if(!asix1.connected && fnet_dhcp_cln_is_enabled(dhcp_desc)){
    Serial.println("DHCP Released");
    fnet_dhcp_cln_release(dhcp_desc);
    fnet_memset_zero(dhcp_desc, sizeof(dhcp_desc));
    bench_srv_release();
    fnet_release();
  }
  else if(!asix1.connected){
//    Serial.println("DHCP already released!");
  }
}

void handleOutput(fnet_netif_t *netif, fnet_netbuf_t *nb) { //Called when a message is sent
  if(nb && (nb->total_length >= FNET_ETH_HDR_SIZE)){
    uint8_t* p = (uint8_t*)sbuf;
    _fnet_netbuf_to_buf(nb, 0u, FNET_NETBUF_COPYALL, p);

    if(nb->total_length >= 12){
//      Serial.print("Message Transmitted: ");
//      Serial.println(nb->total_length);
//      Serial.print("QueuedBefore: ");
//      Serial.println(asix1.txQueued());
//      const uint8_t* end = p + nb->total_length;
//      while(p < end){
//        if(*p <= 0x0F) Serial.print("0");
//        Serial.print(*p, HEX);
//        Serial.print(" ");
//        p++;
//      }
//      Serial.println();
//    p = (uint8_t*)sbuf;
    }
    myusb.Task();
    asix1.sendPacket(p, nb->total_length);
//    Serial.print("QueuedAfter: ");
//    Serial.println(asix1.txQueued());
    myusb.Task();
  }
}

void handleWait() {
  myusb.Task();
}

uint32_t _totalLength;
int32_t _remainingLength = 0;
uint8_t* _lastIndex;
uint8_t* _rxEnd;
uint8_t* _rxStart;
void handleRecieve(const uint8_t* data, uint32_t length) { //Called when ASIX gets a message
  if(length == 0) return;
  RECIEVE:
  if(_remainingLength < 0){
    Serial.println("Remaining");
    while(_lastIndex < _rxEnd){
      *_lastIndex = *data;
      data++;
      _lastIndex++;
    }
    _fnet_eth_input(&fnet_usb0_if, (uint8_t*)rbuf, _totalLength);
    _remainingLength = abs(_remainingLength);
    data += _remainingLength;
    _remainingLength = length - ((_remainingLength + 3) & 0xFFFFFFFC);
    length -= _remainingLength;
    if(length) {
      goto RECIEVE;
    }
  }
  else if(((data[0] | data[2]) == 0xFF) && ((data[1] | data[3]) == 0xFF)) { //Check for header
    _lastIndex = (uint8_t*)rbuf;
    _totalLength = (data[1] << 8) | data[0];
    _remainingLength = length - ((_totalLength + 9) & 0xFFFFFFFC);
    if(_remainingLength < 0){
//      Serial.print("Remaining: ");
//      Serial.println(_remainingLength);
      const uint8_t* end = data + length;
      _rxEnd = (uint8_t*)rbuf + _totalLength;
      data += 6;
      while(data < end){
        *_lastIndex = *data;
        data++;
        _lastIndex++;
      }
    }
    else {
      data += 6;
      _fnet_eth_input(&fnet_usb0_if, (uint8_t*)data, _totalLength);
      if(_remainingLength) {        
//        Serial.println("Remaining data");
//        Serial.print("length: ");
//        Serial.print(length);
//        Serial.print("  rlength: ");
//        Serial.println(_remainingLength);
        data -= 6;
        data += ((_totalLength + 9) & 0xFFFFFFFC);
        length -= ((_totalLength + 9) & 0xFFFFFFFC);
//        asix1.print_hexbytes((uint8_t*)data, ((_totalLength + 9) & 0xFFFFFFFC));
        goto RECIEVE;
      }
    }
  }
  else if(length == abs(_remainingLength)) return; //Technically an error and I don't know where it's happening, but this poses no problems so it's fine
  else{ //Error edgecase unknown cause
    Serial.println("Message Recieve Error, searching for next header");
    Serial.print("length: ");
    Serial.print(length);
    Serial.print("  rlength: ");
    Serial.println(_remainingLength);
    return;
  }
}

void handleSetMACAddress(uint8_t * hw_addr) { //Gets called on initialization
  Serial.print("SetMACAddress: ");
  for(uint8_t i = 0; i < 6; i++) {
    if(hw_addr[i] <= 0x0F) Serial.print("0");
    Serial.print(hw_addr[i], HEX);
  }
  Serial.println();
}

void handleGetMACAddress(fnet_mac_addr_t * hw_addr) { //Gets called everytime a message is sent
  (*hw_addr)[0] = asix1.nodeID[0];
  (*hw_addr)[1] = asix1.nodeID[1];
  (*hw_addr)[2] = asix1.nodeID[2];
  (*hw_addr)[3] = asix1.nodeID[3];
  (*hw_addr)[4] = asix1.nodeID[4];
  (*hw_addr)[5] = asix1.nodeID[5];
  MacAddress[0] = asix1.nodeID[0];
  MacAddress[1] = asix1.nodeID[1];
  MacAddress[2] = asix1.nodeID[2];
  MacAddress[3] = asix1.nodeID[3];
  MacAddress[4] = asix1.nodeID[4];
  MacAddress[5] = asix1.nodeID[5];
//  Serial.print("GetMACAddress: ");
//  for(uint8_t i = 0; i < 6; i++) {
//    if(hw_addr[i] <= 0x0F) Serial.print("0");
//    Serial.print(hw_addr[i], HEX);
//  }
//  Serial.println();
}

void handlePHYRead(fnet_uint32_t reg_addr, fnet_uint16_t *data) { //Could be called, don't think it works correctly
  asix1.readPHY(reg_addr, data);
  Serial.print("PHYRead: ");
  Serial.print(reg_addr, HEX);
  Serial.print("  ");
  Serial.println(*data, HEX);
}

void handlePHYWrite(fnet_uint32_t reg_addr, fnet_uint16_t data) { //Could be called, might work correctly
  asix1.writePHY(reg_addr, data);
  Serial.println("PHYWrite");
}

uint8_t mcHashTable[8] = {0,0,0,0,0,0,0,0}; //Whole table needs resent when one multicast address is changed
void handleMulticastJoin(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr) { //Called when joining multicast group
  //Not setup yet
  Serial.print("MulticastJoin: ");
  for(uint8_t i = 0; i < 6; i++) {
    if(multicast_addr[i] <= 0x0F) Serial.print("0");
    Serial.print(multicast_addr[i], HEX);
  }
  Serial.println();
  uint8_t crc = byteReverse((uint8_t)(fnet_usb_crc_hash(multicast_addr) & 0x000000FF)) >> 2;

  crc &= 0x3F;
  mcHashTable[crc >> 3] |= 1 << (crc & 7);
  asix1.setMulticast((uint8_t*)&mcHashTable);
//  Serial.println("MulticastTable: ");
//  for(uint8_t i = 0; i < 8; i++) {
//    Serial.println(mcHashTable[i],BIN);
//  }
}

void handleMulticastLeave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr) { //Called when leaving multicast group
  //Not setup yet
  Serial.println("MulticastLeave: ");
  for(uint8_t i = 0; i < 6; i++) {
    if(multicast_addr[i] <= 0x0F) Serial.print("0");
    Serial.print(multicast_addr[i], HEX);
  }
  Serial.println();
  uint8_t crc = byteReverse((uint8_t)(fnet_usb_crc_hash(multicast_addr) & 0x000000FF)) >> 2;
  crc &= 0x3F;
  mcHashTable[crc >> 3] ^= 1 << (crc & 7);
  asix1.setMulticast((uint8_t*)&mcHashTable);
//  Serial.println("MulticastTable: ");
//  for(uint8_t i = 0; i < 8; i++) {
//    Serial.println(mcHashTable[i],BIN);
//  }
}

uint8_t byteReverse(uint8_t x) { 
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa); 
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc); 
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0); 
   return x;    
}

fnet_bool_t handleIsConnected() {
  return asix1.connected ? FNET_TRUE : FNET_FALSE;
}

fnet_bench_srv_params_t bench_srv_params;
fnet_bench_srv_desc_t bench_srv_desc;
void bench_srv_init(){
    /* Set Benchmark server parameters.*/
    fnet_memset_zero(&bench_srv_params, sizeof(bench_srv_params));
    bench_srv_params.type = SOCK_STREAM; /* TCP by default */
        if(current_netif){ /* Only on one interface */
            bench_srv_params.address.sa_scope_id = fnet_netif_get_scope_id(current_netif);
        }

        /* Start Benchmark server. */
        bench_srv_desc = fnet_bench_srv_init(&bench_srv_params);
        if(bench_srv_desc){
            /* Instal callbacks */
//            fnet_bench_srv_set_callback_session_begin (fapp_bench_srv_desc, fapp_bench_srv_callback_session_begin, shell_desc);
            fnet_bench_srv_set_callback_session_end (bench_srv_desc, bench_srv_callback_session_end, bench_srv_desc);
        }
}

void bench_srv_release(void){
    fnet_bench_srv_release(bench_srv_desc);
    bench_srv_desc = 0;
}

static void bench_srv_callback_session_end(fnet_bench_srv_desc_t desc, const struct fnet_bench_srv_result *bench_srv_result, void *cookie) {
  if(bench_srv_result){
    uint64_t totalBytes = (bench_srv_result->megabytes * 1000000) + bench_srv_result->bytes;
//    Serial.println("Benchmark results:");
    Serial.print("Megabytes: ");
    Serial.print(totalBytes / 1000000.0, 6);
    Serial.print("  Seconds: ");
    Serial.print(bench_srv_result->time_ms/1000.0, 4);
//    Serial.print("Bytes/Sec: ");
//    Serial.println(totalBytes/(bench_srv_result->time_ms/1000.0), 4);
//    Serial.print("KBytes/Sec: ");
//    Serial.println((totalBytes/(bench_srv_result->time_ms/1000.0))/1000.0, 4);
    Serial.print("  KBits/Sec: ");
    Serial.println((((totalBytes/(bench_srv_result->time_ms/1000.0))/1000.0)*8), 4);
//    Serial.println();
  }
}

fnet_bench_cln_desc_t teensy_bench_cln_desc;
void bench_cln_cmd(uint8_t argc, char* argv){
    fnet_bench_cln_params_t         bench_cln_params;
    fnet_netif_desc_t               netif = current_netif;
    fnet_bench_cln_desc_t           bench_cln_desc;
    fnet_char_t                     ip_str[FNET_IP_ADDR_STR_SIZE_MAX];
    uint8_t i;

    /* Set Benchmark client parameters.*/
    fnet_memset_zero(&bench_cln_params, sizeof(bench_cln_params));

    /* Default values */
    bench_cln_params.type = SOCK_STREAM; /* TCP by default */
    bench_cln_params.message_size = 1272;     /* Default message size */
    bench_cln_params.message_number = 10000; /* Default number of messages */

    /* -a <remote ip> [tcp|udp] [-m <message size>] [-mn <number of messages>] */
    for(i = 0; i < (argc); i++){
        
        if (!fnet_strcmp(&argv[i], "-a")){ /*[-a <remote ip>] */
            i += 3;
            if(i < argc){
                if(fnet_inet_ptos(&argv[i], &bench_cln_params.address) == FNET_ERR){
                    goto ERROR_PARAMETER;
                }
                while(argv[i] != '\0'){
                  i++;
                }
            }
            else{
                goto ERROR_PARAMETER;
            }
        }
        else if (!fnet_strcmp(&argv[i], "tcp")){ /* TCP */
            bench_cln_params.type = SOCK_STREAM;
            while(argv[i] != '\0'){
              i++;
            }
        }
        else if (!fnet_strcmp(&argv[i], "udp")){ /* udp */
            bench_cln_params.type = SOCK_DGRAM;
            while(argv[i] != '\0'){
              i++;
            }
        }
        else if (!fnet_strcmp(&argv[i], "-m")){ /* [-m <message size>] */
            fnet_char_t *p = 0;

            i += 3;
            if(i < argc){
                bench_cln_params.message_size = fnet_strtoul(&argv[i], &p, 0);
                if (bench_cln_params.message_size == 0){
                    goto ERROR_PARAMETER;
                }
            }
            else{
                goto ERROR_PARAMETER;
            }
            while(argv[i] != '\0'){
              i++;
            }
        }
        else if (!fnet_strcmp(&argv[i], "-mn")){ /* [-mn <number of messages>] */
            fnet_char_t *p = 0;

            i += 4;
            if(i < argc){
                bench_cln_params.message_number = fnet_strtoul(&argv[i], &p, 0);
                if (bench_cln_params.message_number == 0){
                    goto ERROR_PARAMETER;
                }
            }
            else{
                goto ERROR_PARAMETER;
            }
            while(argv[i] != '\0'){
              i++;
            }
        }
        else if (argv[i] == '\0'){
        
        }
        else{/* Wrong parameter.*/
            goto ERROR_PARAMETER;
        }
    }

    if(fnet_socket_addr_is_unspecified(&bench_cln_params.address) == FNET_TRUE){ /* Address is not set. */
        Serial.println("Error: <remote ip> is not set");
    }
    else{
        if(netif){ /* Only on one interface */
            bench_cln_params.address.sa_scope_id = fnet_netif_get_scope_id(netif);
        }

        bench_cln_params.callback = bench_cln_callback_session_end;    /* Callback function.*/

        /* Run Benchmark client. */
        bench_cln_desc = fnet_bench_cln_init(&bench_cln_params);
        if(bench_cln_desc){
            teensy_bench_cln_desc = bench_cln_desc;
//
            Serial.println("Benchmark client started.");
            Serial.print("Protocol: ");
            Serial.println((bench_cln_params.type == SOCK_STREAM) ? "TCP" : "UDP");
            Serial.print("Remote IP Addr: ");
            Serial.println(fnet_inet_ntop(bench_cln_params.address.sa_family, bench_cln_params.address.sa_data, ip_str, sizeof(ip_str)));
            Serial.print("Remote Port: ");
            Serial.println(FNET_NTOHS(FNET_CFG_BENCH_CLN_PORT));
            Serial.print("Message Size: ");
            Serial.println(bench_cln_params.message_size);
            Serial.print("Num. of messages: ");
            Serial.println(bench_cln_params.message_number);
            Serial.println();
        }
        else{
            Serial.println("Benchmark client failed to start.");
        }
    }
    return;

ERROR_PARAMETER:
    Serial.print("Bad parameter: ");
    Serial.println(&argv[i]);
    return;
}

static void bench_cln_callback_session_end(fnet_bench_cln_desc_t bench_cln_desc, const fnet_bench_cln_result_t *bench_cln_result, void *cookie){
  if(bench_cln_result){
//        fapp_bench_print_results (shell_desc, bench_cln_result->megabytes, bench_cln_result->bytes, bench_cln_result->time_ms);
      uint64_t totalBytes = (bench_cln_result->megabytes * 1000000) + bench_cln_result->bytes;
//    Serial.println("Benchmark results:");
    Serial.print("Megabytes: ");
    Serial.print(totalBytes / 1000000.0, 6);
    Serial.print("  Seconds: ");
    Serial.print(bench_cln_result->time_ms/1000.0, 4);
    Serial.print("  KBits/Sec: ");
    Serial.println((((totalBytes/(bench_cln_result->time_ms/1000.0))/1000.0)*8), 4);
  }
}

void serialParser(uint8_t argc, char* argv){
  for(uint8_t i = 0; i < (argc) /*avoid the last parameter.*/; i++){
    if (!fnet_strcmp(&argv[i], "benchtx")){ /*benchtx */
      i++;
      if(i < argc){
        argv += 8; //Advance length of command + 1
        argc -= 8; //Decrease length same amount
        Serial.println("Starting benchtx...");
        bench_cln_cmd(argc, argv);
        return;
      }
      else{
        goto ERROR_PARAMETER;
      }
    }
    else     if (!fnet_strcmp(&argv[i], "benchrx")){ /*benchrx */
      i++;
      if(i < argc){
        argv += 8; //Advance length of command + 1
        argc -= 8; //Decrease length same amount
//        bench_cln_cmd(argc, argv);
        Serial.println("Starting benchrx...");
        return;
      }
      else{
        goto ERROR_PARAMETER;
      }
    }
    else{/* Wrong parameter.*/
      goto ERROR_PARAMETER;
    }
  }
  return;

ERROR_PARAMETER:
    Serial.println("Invalid command");
    return;
}
