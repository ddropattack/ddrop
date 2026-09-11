//#define STATS  //Print how many times each thread loops per second

#include <USBHost_t36.h> //USB Host Driver
#include <ASIXEthernet.h> //USB Ethernet Driver
#include <TeensyThreads.h>  //Multi-thread

#include <fnet.h> //TCP/IP stack
#include <port/fnet_usb.h>  //USB settings

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
ASIXEthernet asix1(myusb);

volatile uint8_t rbuf[1024*2]; // recieve buffer
volatile uint8_t sbuf[2000]; // send buffer
volatile uint16_t sbuflen; // length of data to send
uint8_t MacAddress[6] = {0x00,0x50,0xB6,0xBE,0x8B,0xB4}; //Not setup yet, but can't be 0
                                          //Uses your adapters builtin address right now

#ifdef STATS
elapsedMillis advertise;
uint32_t Looped;
volatile uint32_t LoopedUSB;
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
static DMAMEM fnet_uint8_t         stack_heap[(128U * 1024U)];

void setup() {
  Serial.begin(115200);
  // Wait 1.5 seconds before turning on USB Host.  If connected USB devices
  // use too much power, Teensy at least completes USB enumeration, which
  // makes isolating the power issue easier.
  delay(1500);
  Serial.println("USB Host InputFunctions example");
  delay(10);
  myusb.begin();
  asix1.setHandleRecieve(handleRecieve);
  asix1.setHandleWait(handleWait);
  Serial.println("USB Ready");
//  threads.addThread(usbpoll);
  threads.addThread(usbthread);
  //All of these have to be set to function correctly
  setHandleOutput(handleOutput);
  setHandleSetMACAddress(handleSetMACAddress);
  setHandleGetMACAddress(handleGetMACAddress);
  setHandlePHYRead(handlePHYRead);
  setHandlePHYWrite(handlePHYWrite);
  setHandleMulticastLeave(handleMulticastLeave);
  setHandleMulticastJoin(handleMulticastJoin);
  setHandleIsConnected(handleIsConnected);
}

void loop() {
  while(1){
    myusb.Task();
    asix1.read();
    if(uint8_t index = Serial.available()){
      if(index >= 64) break;
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
    if(advertise >= 1000) {
      advertise -= 1000;
      Serial.print("Looped: ");
      Serial.print(Looped);
      Serial.print("  LoopedUSB: ");
      Serial.print(LoopedUSB);
      if(fnet_netif_is_initialized(current_netif)){
        Serial.print("  FNETMemFree: ");
        Serial.print((uint32_t)fnet_free_mem_status());
      }
      Serial.print("  LinkSpeed: ");
      Serial.println(asix1.PHYSpeed ? "100BASE" : "10BASE");
      Looped = 0;
      LoopedUSB = 0;
    }
#endif
  }
}
