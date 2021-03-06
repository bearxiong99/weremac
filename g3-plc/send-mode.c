/* Copyright (c) 2017, David Hauweele <david@hauweele.net>
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

#ifdef __linux__
# define _POSIX_C_SOURCE 199309L
#endif /* __linux__ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "time-substract.h"
#include "g3-plc/g3plc-str.h"
#include "scale.h"
#include "mode.h"
#include "help.h"
#include "dump.h"
#include "common.h"

static int display_time;
static const char *message = "Hello World!";

static void cb_recv(const struct g3plc_data_hdr *hdr,
                    const void *payload, unsigned payload_size,
                    int status, void *data)
{
  /* This mode only sends so we ignored received frames. */
  UNUSED(hdr);
  UNUSED(payload);
  UNUSED(payload_size);
  UNUSED(status);
  UNUSED(data);
}

static void init(const struct context *ctx, struct g3plc_config *g3plc)
{
  UNUSED(ctx);
  g3plc->callbacks.cb_recv = cb_recv;
}

static void start(const struct context *ctx)
{
  struct timespec begin, end;
  uint64_t nsec;
  int ret;

  UNUSED(ctx);

  clock_gettime(CLOCK_MONOTONIC, &begin);
  ret = g3plc_send(ctx->dst_mac, message, strlen(message));
  clock_gettime(CLOCK_MONOTONIC, &end);

  nsec = substract_nsec(&begin, &end);

  putchar('\n');
  if(display_time)
    printf("TIME     : %s\n", scale_time(nsec));
  printf("TX STATUS: %s (%d)\n", g3plc_send2str(ret), ret);
}

static void destroy(const struct context *ctx)
{
  UNUSED(ctx);
}

static int parse_option(const struct context *ctx, int c)
{
  UNUSED(ctx);

  switch(c) {
  case 'T':
    display_time = 1;
    return 1;
  case 'm':
    message = optarg;
    return 1;
  }

  return 0;
}

struct option send_opts[] = {
  { "time", no_argument, NULL, 'T' },
  { "message", required_argument, NULL, 'm' },
  { NULL, 0, NULL, 0 }
};
struct opt_help send_messages[] = {
  { 'T', "time",    "Display the time necessary to send the message (including retransmissions)" },
  { 'm', "message", "Message to be send (default: \"Hello World!\")"},
  { 0, NULL, NULL }
};

struct iface_mode iface_mode = {
  .name = "send",
  .description = "Send a single frame",

  .optstring      = "Tm:",
  .long_opts      = send_opts,
  .extra_messages = send_messages,
  .parse_option   = parse_option,

  .init    = init,
  .destroy = destroy,
  .start   = start
};
