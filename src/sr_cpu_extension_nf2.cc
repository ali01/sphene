/*-----------------------------------------------------------------------------
 * file:  sr_cpu_extension_nf2.c
 * date:  Mon Feb 09 16:58:30 PST 2004
 * Author: Martin Casado
 *
 * 2007-Apr-04 04:57:55 AM - Modified to support NetFPGA v2.1 /mc
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#include "sr_cpu_extension_nf2.h"

#include "sr_base_internal.h"

#include "sr_vns.h"

#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <vector>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_plane.h"
#include "fwk/scoped_lock.h"
#include "interface.h"
#include "interface_map.h"
#include "router.h"

using std::string;
using std::vector;

struct sr_ethernet_hdr
{
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6
#endif
    uint8_t  ether_dhost[ETHER_ADDR_LEN];    /* destination ethernet address */
    uint8_t  ether_shost[ETHER_ADDR_LEN];    /* source ethernet address */
    uint16_t ether_type;                     /* packet type ID */
} __attribute__ ((packed)) ;

static char*    copy_next_field(FILE* fp, char*  line, char* buf);
static uint32_t asci_to_nboip(const char* ip);
static void     asci_to_ether(const char* addr, uint8_t mac[6]);


/*-----------------------------------------------------------------------------
 * Method: sr_cpu_init_hardware(..)
 * scope: global
 *
 * Read information for each of the router's interfaces from hwfile
 *
 * format of the file is (1 interface per line)
 *
 * <name ip mask hwaddr>
 *
 * e.g.
 *
 * eth0 192.168.123.10 255.255.255.0 ca:fe:de:ad:be:ef
 *
 *---------------------------------------------------------------------------*/


int sr_cpu_init_hardware(struct sr_instance* sr, const char* hwfile)
{
    struct sr_vns_if vns_if;
    FILE* fp = 0;
    char line[1024];
    char buf[SR_NAMELEN];
    char *tmpptr;


    if ( (fp = fopen(hwfile, "r") ) == 0 )
    {
        fprintf(stderr, "Error: could not open cpu hardware info file: %s\n",
                hwfile);
        return -1;
    }

    Debug(" < -- Reading hw info from file %s -- >\n", hwfile);
    while ( fgets( line, 1024, fp) )
    {
        line[1023] = 0; /* -- insurance :) -- */

        /* -- read interface name into buf -- */
        if(! (tmpptr = copy_next_field(fp, line, buf)) )
        {
            fclose(fp);
            fprintf(stderr, "Bad formatting in cpu hardware file\n");
            return 1;
        }
        Debug(" - Name [%s] ", buf);
        strncpy(vns_if.name, buf, SR_NAMELEN);
        /* -- read interface ip into buf -- */
        if(! (tmpptr = copy_next_field(fp, tmpptr, buf)) )
        {
            fclose(fp);
            fprintf(stderr, "Bad formatting in cpu hardware file\n");
            return 1;
        }
        Debug(" IP [%s] ", buf);
        vns_if.ip = asci_to_nboip(buf);

        /* -- read interface mask into buf -- */
        if(! (tmpptr = copy_next_field(fp, tmpptr, buf)) )
        {
            fclose(fp);
            fprintf(stderr, "Bad formatting in cpu hardware file\n");
            return 1;
        }
        Debug(" Mask [%s] ", buf);
        vns_if.mask = asci_to_nboip(buf);

        /* -- read interface hw address into buf -- */
        if(! (tmpptr = copy_next_field(fp, tmpptr, buf)) )
        {
            fclose(fp);
            fprintf(stderr, "Bad formatting in cpu hardware file\n");
            return 1;
        }
        Debug(" MAC [%s]\n", buf);
        asci_to_ether(buf, vns_if.addr);

        sr_integ_add_interface(sr, &vns_if);

    } /* -- while ( fgets ( .. ) ) -- */
    Debug(" < --                         -- >\n");

    fclose(fp);
    return 0;

} /* -- sr_cpu_init_hardware -- */

/*-----------------------------------------------------------------------------
 * Method: sr_cpu_input(..)
 * Scope: Local
 *
 *---------------------------------------------------------------------------*/

int sr_cpu_input(struct sr_instance* sr)
{
  uint8_t buf[4096];

  /* REQUIRES */
  assert(sr);

  /* Set of file descriptors for watch for reading. */
  fd_set rfds;
  FD_ZERO(&rfds);

  Router::Ptr router = sr->router;
  InterfaceMap::Ptr if_map = router->dataPlane()->interfaceMap();
  vector<int> fds_vec;
  vector<string> names;
  {
    Fwk::ScopedLock<InterfaceMap> lock(if_map);
    InterfaceMap::iterator it;
    for (it = if_map->begin(); it != if_map->end(); ++it) {
      Interface::Ptr iface = it->second;
      int fd = iface->socketDescriptor();
      if (fd > 0) {
        FD_SET(fd, &rfds);
        fds_vec.push_back(fd);
        names.push_back(iface->name());
      }
    }
  }

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  int ret = select(fds_vec.size(), &rfds, NULL, NULL, &tv);
  if (ret == -1) {
    perror("select()");
    return 0;
  } else if (ret) {
    vector<int>::iterator it;
    for (uint i = 0; i < fds_vec.size(); ++i) {
      int fd = fds_vec[i];

      if (!FD_ISSET(fd, &rfds))
        continue;

      /* Read data from ready file descriptor. */
      int bytes = read(fd, buf, sizeof(buf));
      if (bytes <= 0) {
        perror("read()");
        return 0;
      }

      /* Send packet through processing pipeline. */
      sr_integ_input(sr, buf, bytes, names[i].c_str());
      break;
    }
  } else {
    /*fprintf(stdout, "timeout reading data\n");*/
  }

  return 1;
}


/*-----------------------------------------------------------------------------
 * Method: sr_cpu_output(..)
 * Scope: Global
 *
 * Returns the length of the packet on success, -1 on failure.
 *---------------------------------------------------------------------------*/

int sr_cpu_output(struct sr_instance* const sr /* borrowed */,
                  uint8_t* const buf /* borrowed */ ,
                  const unsigned int len,
                  const char* const iface_name /* borrowed */)
{
  /* REQUIRES */
  assert(sr);
  assert(buf);
  assert(iface_name);

  Router::Ptr router = sr->router;
  InterfaceMap::Ptr if_map = router->dataPlane()->interfaceMap();
  Interface::Ptr iface = if_map->interface(iface_name);

  // TODO(ms): Really need some real logging here.
  if (!iface)
    return -1;

  int fd = iface->socketDescriptor();
  if (fd < 0)
    return -1;

  int written = write(fd, buf, len);
  return written;

  return -1;
}


/*-----------------------------------------------------------------------------
 * Method: copy_next_field(..)
 * Scope: Local
 *
 *---------------------------------------------------------------------------*/

static
char* copy_next_field(FILE* fp, char* line, char* buf)
{
    char* tmpptr = buf;
    while ( *line  && isspace((int)*line)) /* -- XXX: potential overrun here */
    { line++; }
    if(! *line )
    { return 0; }
    while ( *line && ! isspace((int)*line) && ((tmpptr - buf) < SR_NAMELEN))
    { *tmpptr++ = *line++; }
    *tmpptr = 0;
    return line;
} /* -- copy_next_field -- */

/*-----------------------------------------------------------------------------
 * Method: asci_to_nboip(..)
 * Scope: Local
 *
 *---------------------------------------------------------------------------*/

static uint32_t asci_to_nboip(const char* ip)
{
    struct in_addr addr;

    if ( inet_pton(AF_INET, ip, &addr) <= 0 )
    { return 0; } /* -- 0.0.0.0 unsupported so its ok .. yeah .. really -- */

    return addr.s_addr;
} /* -- asci_to_nboip -- */

/*-----------------------------------------------------------------------------
 * Method: asci_to_ether(..)
 * Scope: Local
 *
 * Look away .. please ... just look away
 *
 *---------------------------------------------------------------------------*/

static void asci_to_ether(const char* addr, uint8_t mac[6])
{
    uint32_t tmpint;
    const char* buf = addr;
    int i = 0;
    for( i = 0; i < 6; ++i )
    {
        if (i)
        {
            while (*buf && *buf != ':')
            { buf++; }
            buf++;
        }
        sscanf(buf, "%x", &tmpint);
        mac[i] = tmpint & 0x000000ff;
    }
} /* -- asci_to_ether -- */
