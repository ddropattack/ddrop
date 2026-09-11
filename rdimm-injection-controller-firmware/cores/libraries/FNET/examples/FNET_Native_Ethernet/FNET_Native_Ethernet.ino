#define STATS  //Print how many times each thread loops per second

#include <TeensyThreads.h>  //Multi-thread

#include <fnet.h> //TCP/IP stack
#include <port/fnet_usb.h>  //USB settings
//#include <stack/fnet_netif_prv.h"

volatile uint8_t rbuf[1024*2]; // recieve buffer
volatile uint8_t sbuf[2000]; // send buffer
volatile uint16_t sbuflen; // length of data to send
uint8_t MacAddress[6] = {0x04,0xe9,0xe5,0x00,0x00,0x01}; //Not setup yet, but can't be 0
                                          //Uses your adapters builtin address right now

#ifdef STATS
elapsedMillis advertise;
uint32_t LoopedPoll;
volatile uint32_t Looped;
#endif
Threads::Mutex fnet_mutex1; //Used by the stack
uint16_t fnet_mutex1_calls; //Used to support recursiveness
Threads::Mutex fnet_mutex2; //Used by the services
uint16_t fnet_mutex2_calls; //Used to support recursiveness
uint8_t mutex_init_calls = 0; //Change which mutex to assign
fnet_return_t teensy_mutex_init(fnet_mutex_t *mutex) {
  Serial.print("Mutex initialized: ");
  Serial.println(mutex_init_calls);
  
  fnet_return_t result = FNET_ERR;
  if(mutex){
    if(mutex_init_calls == 0)*mutex = (fnet_mutex_t)&fnet_mutex1;
    else if(mutex_init_calls == 1)*mutex = (fnet_mutex_t)&fnet_mutex2;
    mutex_init_calls++;
    result = FNET_OK;
  }
  return result;
}

void teensy_mutex_release(fnet_mutex_t *mutex) {
  Serial.println("Mutex released");
  mutex_init_calls = 0;
  return; //TeensyThreads has no function for this?
}

void teensy_mutex_lock(fnet_mutex_t *mutex) {
//  Serial.print("Mutex locked: ");
//  Serial.println((uint32_t)((Threads::Mutex*)*mutex));
  Threads::Mutex *_mutex = (Threads::Mutex*)*mutex;
  if(&fnet_mutex1 == *mutex) {
    fnet_mutex1_calls++;
    if(fnet_mutex1_calls == 1){
      _mutex->lock();
    }
  }
  else if(&fnet_mutex2 == *mutex) {
    fnet_mutex2_calls++;
    if(fnet_mutex2_calls == 1){
      _mutex->lock();
    }
  }
}

void teensy_mutex_unlock(fnet_mutex_t *mutex) {
//  Serial.print("Mutex unlocked: ");
//  Serial.println((uint32_t)((Threads::Mutex*)*mutex));
  Threads::Mutex *_mutex = (Threads::Mutex*)*mutex;
  if(&fnet_mutex1 == *mutex) {
    fnet_mutex1_calls--;
    if(fnet_mutex1_calls == 0){
      _mutex->unlock();
    }
  }
  else if(&fnet_mutex2 == *mutex) {
    fnet_mutex2_calls--;
    if(fnet_mutex2_calls == 0){
      _mutex->unlock();
    }
  }
}

const fnet_mutex_api_t teensy_mutex_api ={
    .mutex_init = teensy_mutex_init,
    .mutex_release = teensy_mutex_release,
    .mutex_lock = teensy_mutex_lock,
    .mutex_unlock = teensy_mutex_unlock,
};

fnet_time_t timer_get_ms(void){ //Used for multi-thread version
    fnet_time_t result;
    result =  millis();
    return result;
}
fnet_dhcp_cln_params_t dhcp_params; //DHCP intialization parameters
fnet_dhcp_cln_desc_t dhcp_desc; //DHCP object
fnet_netif_desc_t current_netif; //Network interface, USB Ethernet
static DMAMEM fnet_uint8_t         stack_heap[(160U * 1024U)];

void setup() {
  while(!Serial);
  delay(1500);
//  NonNativeSetup();
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
      if(fnet_netif_init(FNET_CPU_ETH0_IF, MacAddress, 6) != FNET_ERR){
        Serial.println("netif Initialized");
        if((current_netif = fnet_netif_get_default()) == 0){
          Serial.println("ERROR: Network Interface is not configurated!");
        }
        else {
          Serial.println("SUCCESS: Network Interface is configurated!");
        }
      }
      else {
        Serial.println("Error:netif initialization failed.\n");
      }
    }
    else {
      Serial.println("Error:TCP/IP stack initialization failed.\n");
    }

  threads.addThread(pollthread);
}

void loop() {
  while(1){
    if(uint8_t index = Serial.available()){
      if(index >= 64) return;
      char serialMessage[64];
      for(uint8_t i = 0;i<index;i++){
        serialMessage[i] = Serial.read();
        if(serialMessage[i] == ' '){ //Each command needs to be null-terminated
          serialMessage[i] = '\0';
        }
        else if(serialMessage[i] == '\n'){ //Each command needs to be null-terminated
          serialMessage[i] = '\0';
        }
      }
      serialParser(index, (char*)&serialMessage);
    }
#ifdef STATS
    Looped++;
    
//    printStats();
#endif
  }
}

void printStats() {
  if(advertise >= 1000) {
      advertise -= 1000;
      Serial.print("Looped: ");
      Serial.print(Looped);
      Serial.print("  LoopedPoll: ");
      Serial.print(LoopedPoll);
      if(fnet_netif_is_initialized(current_netif)){
        Serial.print("  FNETMemFree: ");
        Serial.print((uint32_t)fnet_free_mem_status());
      }
      Serial.print("  Link: ");
      Serial.print(linkStatus());
      Serial.println();
      Looped = 0;
      LoopedPoll = 0;
    }
}
