/* osc_handlers.c
This file is a part of 'silica_display'
This program is free software: you can redistribute it and/or modify
hit under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

'silica_display' is a JACK client for learning about software synthesis

Copyright 2015 murray foster */

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);

#include "osc.h"
#include "osc_handlers.h"
#include "silica_displaylib_handlers.h"

void osc_error(int num, const char *msg, const char *path)
{
  printf("liblo server error %d in path %s: %s\n", num, path, msg);
  fflush(stdout);
}

/* for debugging,
 * catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
  int i;
  
  printf("path: <%s>\n", path);
  for (i = 0; i < argc; i++) {
    printf("arg %d '%c' ", i, types[i]);
    lo_arg_pp((lo_type)types[i], argv[i]);
    printf("\n");
  }
  return 0;
}

int osc_address_handler(const char *path, const char *types, lo_arg **argv,
			int argc, void *data, void *user_data)
{
  printf("path: %s\n", path);
  char *new_host_out = argv[0];
  char *new_port_out = argv[1];
  printf("hit osc_address_handler\n");
  osc_change_address(new_host_out, new_port_out);
  return 0;
} /* osc_address_handler */

int osc_send_startup_system_process_handler(const char *path, const char *types, lo_arg **argv,
					    int argc, void *data, void *user_data)
{
  char *procname = argv[0];
  int status = argv[1]->i;
  silica_display_handler_send_startup_system_process(procname, status);

  
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/startup_system_process", "si", procname, status);
  free(lo_addr_send);
  return 0;  
} /* osc_send_startup_system_process_handler */

int osc_send_blank_screen_handler(const char *path, const char *types, lo_arg **argv,
					    int argc, void *data, void *user_data)
{
  printf("calling send_blank_screen()\n");
  silica_display_handler_send_blank_screen(); 
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/blank_screen", NULL);
  free(lo_addr_send);
  return 0;  
} /* osc_send_blank_screen_handler */

int osc_send_status_cpu_handler(const char *path, const char *types, lo_arg **argv,
					    int argc, void *data, void *user_data)
{
  float cpu0_idle = argv[0]->f;
  float cpu1_idle = argv[1]->f;
  float cpu2_idle = argv[2]->f;
  float cpu3_idle = argv[3]->f;

  float total_idle = cpu0_idle+cpu1_idle+cpu2_idle+cpu3_idle;
  
  silica_display_handler_send_status_cpu(400.0 - total_idle, 100.0 - cpu0_idle, 100.0 - cpu1_idle, 100.0 - cpu2_idle, 100.0 - cpu3_idle);
  
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/stats/cpu", "ffff", cpu0_idle, cpu1_idle, cpu2_idle, cpu3_idle);
  free(lo_addr_send);
  return 0;  
} /* osc_send_status_cpu_handler */

int osc_send_status_mem_handler(const char *path, const char *types, lo_arg **argv,
			       int argc, void *data, void *user_data)
{
  float mem = argv[0]->f;
  float total = argv[1]->f;
  silica_display_handler_send_status_mem(mem, total);
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/stats/mem", "ff", mem, total);
  free(lo_addr_send);
  return 0;  
} /* osc_send_status_mem_handler */

int osc_send_status_disk_handler(const char *path, const char *types, lo_arg **argv,
				int argc, void *data, void *user_data)
{
  float disk = argv[0]->f;
  float total = argv[1]->f;
  silica_display_handler_send_status_disk(disk, total);
  
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/stats/disk", "ff", disk, total);
  free(lo_addr_send);
  return 0;
} /* osc_send_status_disk_handler */

int osc_send_status_network_handler(const char *path, const char *types, lo_arg **argv,
				    int argc, void *data, void *user_data)
{
  int ip0 = argv[0]->i;
  int ip1 = argv[1]->i;
  int ip2 = argv[2]->i;
  int ip3 = argv[3]->i;
  
  silica_display_handler_send_status_network(ip0, ip1, ip2, ip3);
  lo_address lo_addr_send = lo_address_new((const char *)send_host_out, (const char*)send_port_out);
  lo_send(lo_addr_send, "/silica/display/stats/network", "iiii", ip0, ip1, ip2, ip3);
  free(lo_addr_send);
  return 0;
} /* osc_send_status_disk_handler */


char *int_to_str(int x) {
  char *buffer = malloc(sizeof(char) * 13);
  if(buffer)
    sprintf(buffer, "%d", x);
  return buffer;
} /* int_to_str */


