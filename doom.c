//#define DOOM2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <conio.h>
#include <dos.h>

#include "doomnet.h"
//#include "ipxstr.h"
#include "ipx_frch.h" // FRENCH VRESION

doomcom_t doomcom;
int vectorishooked;
void interrupt (*olddoomvect) (void);

/*
================
=
= LaunchDOOM
=
These fields in doomcom should be filled in before calling:

    short numnodes; // console is always node 0
    short ticdup; // 1 = no duplication, 2-5 = dup for slow nets
    short extratics; // 1 = send a backup tic in every packet
    short consoleplayer; // 0-3 = player number
    short numplayers; // 1 = left, 0 = center, -1 = right 
    short drone; // 1 = drone
================
*/

void LaunchDOOM(void)
{
    char *newargs[99];
    char adrstring[10];
    long flatadr;


    // prepare for DOOM
    doomcom.id = DOOMCOM_ID;

// hook the interrup vector
    olddoomvect = getvect (doomcom.intnum);
    setvect (doomcom.intnum,(void interrupt (*)(void))MK_FP(_CS, (int)NetISR));
    vectorishooked = 1;

// build the argument list for DOOM, adding a -net &doomcom
    memcpy (newargs, _argv,(_argc+1)*2);
    newargs[_argc] = "-net";
    flatadr = (long)_DS*16 + (unsigned)&doomcom;
    sprintf (adrsting, "%lu", flatadr);
    newargs[_argc+1] = adrsting;
    newargs[_argc+2] = NULL;

    if(!access("doom2.exe", 0))
        spawnv (P_WAIT, "doom2", newargs);
    else
        spawnv (P_WAIT, "doom", newargs);

    #ifdef DOOM2
    printf (STR_RETURNED"\n");
    #else
    printf ("Returned from DOOM\n");
    #endif

// ipxnet.c

#include <studio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <process.h>
#include <value.h>

#include "ipxnet.h"

/*
=================

IPX PACKET DRIVER

=================

*/

packet_t packet[NUMPACKETS];

nodeadr_t nodeadr[MAXNETNODES+1]; // first is local, last is broadcast

nodeadr_t remoteadr; // set by each GetPacket

localadr_t localadr; // set at startup

extern int socketid;

void far (*IPX)(void);

long localtime; // for time stamp in packets
long remotetime;

// =====

int OpenSocket(short socketNumber)
{
    _DX = socketNumber;
    _BX = 0;
    _AL = 0;
    IPX();
    if (_AL)
        Error ("OpenSocket: 0x%x", _AL);
    return _DX;
}

void CloseSocket(short socketNumber)
{
    _DX = socketNumber;
    _BX = 1;
    IPX();
}

void ListenForPacket(ECB *ecb)
{
    _SI = FP_OFF(ecb);
    _EC = FP_SEG(ecb);
    _BX = 4;
    IPX();
    if(_AL)
        Error ("ListenForPacket: 0x%x", _AL);
}

void GetLocalAddress (void)
{
    _SI = FP_OFF(&localadr);
    _ES = FP_SEG(&localadr);
    _BX = 9;
    IPX();
}

