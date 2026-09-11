void pollthread() {
  elapsedMicros pollTimer;
  Serial.println("Poll Thread Active!");
  while(1){
    if(pollTimer >= 1000){
      pollTimer -= 1000;
      checkLink();
      if(fnet_netif_is_initialized(current_netif)){
        fnet_poll();
      }
      #ifdef STATS
      LoopedPoll++;
      printStats();
      #endif
    }
    else{
      threads.yield();
    }
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
//  Serial.println(linkStatus());
  if(linkStatus() && fnet_dhcp_cln_is_enabled(dhcp_desc)){
//    Serial.println("DHCP already running!");
  }
  else if(linkStatus()){
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
  else if(!linkStatus() && fnet_dhcp_cln_is_enabled(dhcp_desc)){
    Serial.println("DHCP Released");
    fnet_dhcp_cln_release(dhcp_desc);
    fnet_memset_zero(dhcp_desc, sizeof(dhcp_desc));
    bench_srv_release();
//    fnet_release();
  }
  else if(!linkStatus()){
//    Serial.println("DHCP already released!");
  }
}

uint8_t linkStatus(){
  uint16_t reg_data;
  fnet_eth_phy_read(fnet_netif_get_default(), 0x01, &reg_data);
  return (reg_data & (1 << 2)) ? true : false;
}

fnet_bool_t handleIsConnected() {
  return linkStatus() ? FNET_TRUE : FNET_FALSE;
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
