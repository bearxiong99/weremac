/* Copyright (c) 2017, Aurélien Van Laere <aurelien.vanlaere@gmail.com>
                       David Hauweele <david@hauweele.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This is the platform independent part of our G3-PLC driver
   It does not have any code dependent on a specific platform
   (such as S7G2, RPi or Linux), it's just pure C. */

#ifndef _G3PLC_H_
#define _G3PLC_H_

#include <stdint.h>

#define G3PLC_MAJOR 2
#define G3PLC_MINOR 1

enum g3plc_flags {
  LORAMAC_NOACK = 0x1, /* enable ACK communications */
};

/* Initialization status */
enum g3plc_init_status {
  G3PLC_INIT_SUCCESS,
};

struct g3plc_config {
    struct g3plc_callbacks {
    void (*cb_recv)(uint16_t src, uint16_t dst,
                    const void *payload, unsigned payload_size,
                    int status, void *data);
  } callbacks;

  /* The driver will use the uart_send() function to write
     the resulting frame on the device's UART. This
     functions should return a negative value in case of
     error or 0 on success. */
  int  (*uart_send)(const void *buf, unsigned int size);

  /* The function g3plc_uart_putc() is generally called
     from an interrupt handler. Since we cannot parse
     the frame in this handler, we defer processing when
     a complete frame has been received. Control is then
     transferred to this function which can either directly
     be g3plc_recv_frame() or use semaphores to defer
     outside of the interrupt context. */
  int (*recv_frame)(void);

  /* Clear/set the reset pin. */
  void (*reset_clear)(void);
  void (*reset_set)(void);

  /* Not all platform provide byte ordering functions
     with the same names as POSIX. */
  uint16_t (*htons)(uint16_t v);
  uint32_t (*htonl)(uint32_t v);
  uint16_t (*ntohs)(uint16_t v);
  uint32_t (*ntohl)(uint32_t v);

  /* Microseconds sleep. */
  void (*usleep)(unsigned long us);

  uint16_t pan_id;      /* PAN ID */
  uint16_t mac_address; /* device short MAC address */
  unsigned int retrans; /* maximum number of retransmissions */
  unsigned long flags;  /* (see g3plc_flags) */

  void *data; /* context data passed to user callbacks */
};

/* Initialize the LoRaMAC driver (see loramac_config).
   Return 0 on success, for other errror codes see g3plc_init_status. */
int g3plc_init(const struct g3plc_config *conf);

/* Assemble and send a frame to the specified destination using G3PLC.
   When ACK is enabled, this function will block until the packet has
   been successfully transmitted. For the error see g3plc_send_status.
   If the tx pointer is not null, it is replaced with the number of
   transmissions necessary to succesfully send the packet. */
int g3plc_send(uint16_t dst, const void *payload, unsigned int payload_size, unsigned int *tx);

/* Start the processing of a frame. Can be called either automatically
   when a complete frame has been received or manually from another thread. */
int g3plc_recv_frame(void);

/* Called by the platform dependent part of the driver when a character
   has been received on UART from the device. This function can block
   when a full frame has been received. It may also block indefinitely
   if the receive callback itself is blocked. Note that this function
   is *NOT* reentrant. You have to wait for its completion until you
   can call it again. */
int g3plc_uart_putc(unsigned char c);


#endif /* _G3PLC_H_ */
