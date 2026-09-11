/**************************************************************************
*
* Copyright 2018 by Andrey Butok. FNET Community.
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

/********************************************************************/
fnet_return_t fnet_cpu_serial_init(fnet_index_t port_number, fnet_uint32_t baud_rate)
{
    return fnet_mimxrt_serial_init(port_number, baud_rate);
}

/********************************************************************/
void fnet_cpu_serial_putchar (fnet_index_t port_number, fnet_char_t character)
{
    fnet_mimxrt_serial_putchar(port_number, character);
}
/********************************************************************/
fnet_int32_t fnet_cpu_serial_getchar (fnet_index_t port_number)
{
    return fnet_mimxrt_serial_getchar(port_number);
}

#endif /*FNET_MIMXRT*/
