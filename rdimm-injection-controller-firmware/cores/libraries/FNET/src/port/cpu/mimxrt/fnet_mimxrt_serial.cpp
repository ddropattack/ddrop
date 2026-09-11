/**************************************************************************
*
* Copyright 2020 by Tino Hernandez <vjmuzik1@gmail.com>
*
***************************************************************************
*
*  Licensed under the Apache License, Version 2.0 (the "License"); you may
*  not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
*  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
***************************************************************************
*
*  iMX RT Serial port I/O functions.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_MIMXRT

#include "fnet.h"
#include "fnet_mimxrt_serial.h"
#include <Arduino.h>

Stream* Serials[] = {&Serial,
                     &Serial1,
                     &Serial2,
                     &Serial3,
                     &Serial4,
                     &Serial5,
                     &Serial6,
                     &Serial7,
#if defined(ARDUINO_TEENSY41)
                     &Serial8,
#endif
#if defined(CDC2_STATUS_INTERFACE) && defined(CDC2_DATA_INTERFACE)
                     &SerialUSB1,
#endif
#if defined(CDC3_STATUS_INTERFACE) && defined(CDC3_DATA_INTERFACE)
                     &SerialUSB2
#endif
};

fnet_return_t fnet_mimxrt_serial_init(fnet_index_t port_number, fnet_uint32_t baud_rate)
{
    //Serial object should be initialized in user's sketch
    return FNET_OK;
}

/********************************************************************/
void fnet_mimxrt_serial_putchar (fnet_index_t port_number, fnet_char_t character)
{
    if(!(port_number < (sizeof(Serials) / 4))) return;
    Serials[port_number]->write(character);
    Serial.send_now();
}
/********************************************************************/
fnet_int32_t fnet_mimxrt_serial_getchar (fnet_index_t port_number)
{
    if(!(port_number < (sizeof(Serials) / 4))) return FNET_ERR;
    if(Serials[port_number]->available()){
        return Serials[port_number]->read();
    }
    return FNET_ERR;
}


#endif /*FNET_MIMXRT*/
