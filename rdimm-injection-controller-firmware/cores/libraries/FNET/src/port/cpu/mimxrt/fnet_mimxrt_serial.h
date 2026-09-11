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

#ifndef _FNET_MIMXRT_SERIAL_H_

#define _FNET_MIMXRT_SERIAL_H_

#if FNET_MIMXRT


#if defined(__cplusplus)
extern "C" {
#endif

fnet_return_t fnet_mimxrt_serial_init(fnet_index_t port_number, fnet_uint32_t baud_rate);
void fnet_mimxrt_serial_putchar (fnet_index_t port_number, fnet_char_t character);
fnet_int32_t fnet_mimxrt_serial_getchar (fnet_index_t port_number);

#if defined(__cplusplus)
}
#endif


#endif /* FNET_MIMXRT */

#endif /*_FNET_MIMXRT_SERIAL_H_*/
