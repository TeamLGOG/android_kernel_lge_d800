/*
 * Linux 2.6.32 and later Kernel module for VMware MVP Guest Communications
 *
 * Copyright (C) 2010-2012 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; see the file COPYING.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#line 5

/* 
        
  
                                                                   
 */

#include "comm_os.h"
#include "comm_os_mod_ver.h"
#include "comm_svc.h"


/*
                                                                       
                                          
 */

static int Init(void *args);
static void Exit(void);

COMM_OS_MOD_INIT(Init, Exit);

static volatile int running;                 //                         


/* 
                                                      
                                            
                          
                                                  
 */

static int
Init(void *argsIn)
{
   int rc = -1;
   unsigned int maxChannels = 8;

   /*
                                                          
                                           
    */
#if defined(COMM_BUILDING_SERVER)
   unsigned int pollingMillis = (unsigned int)-1;
#else
   unsigned int pollingMillis = 2000;
#endif
   unsigned int pollingCycles = 1;
   const char *args = argsIn;

   if (args && *args) {
      /*                         */
      sscanf(args,
             "max_channels:%u,poll_millis:%u,poll_cycles:%u",
             &maxChannels, &pollingMillis, &pollingCycles);
      CommOS_Debug(("%s: arguments [%s].\n", __FUNCTION__, args));
   }

   rc = Comm_Init(maxChannels);
   if (rc) {
      goto out;
   }

   rc = CommOS_StartIO(COMM_OS_MOD_SHORT_NAME_STRING "-disp",
                       Comm_DispatchAll, pollingMillis, pollingCycles,
                       COMM_OS_MOD_SHORT_NAME_STRING "-aio");
   if (rc) {
      unsigned long long timeout = 0;

      Comm_Finish(&timeout); /*                                         */
      goto out;
   }
   running = 1;
   rc = 0;

out:
   return rc;
}


/* 
                                         
                                                  
 */

static int
Halt(void)
{
   unsigned int maxTries = 10;
   int rc = -1;

   if (!running) {
      rc = 0;
      goto out;
   }

   for ( ; maxTries; maxTries--) {
      unsigned long long timeout = 2000ULL;

      CommOS_Debug(("%s: Attempting to halt...\n", __FUNCTION__));
      if (!Comm_Finish(&timeout)) {
         running = 0;
         rc = 0;
         break;
      }
   }

out:
   return rc;
}


/* 
                                   
                                                                      
 */

static void
Exit(void)
{
   if (!Halt()) {
      CommOS_StopIO();
   }
}


/* 
                                                                           
                                             
                                    
                                               
 */

int
CommSvc_RegisterImpl(const CommImpl *impl)
{
   return Comm_RegisterImpl(impl);
}


/* 
                                                                             
                                             
                                    
 */

void
CommSvc_UnregisterImpl(const CommImpl *impl)
{
   Comm_UnregisterImpl(impl);
}


/* 
                                                                              
                                                                             
                                                        
                                    
                                                 
                                                  
                                                  
                                                                            
 */

int
CommSvc_Alloc(const CommTranspInitArgs *transpArgs,
              const CommImpl *impl,
              int inBH,
              CommChannel *newChannel)
{
   return Comm_Alloc(transpArgs, impl, inBH, newChannel);
}


/* 
                                                                
                                     
                                                 
                                                         
 */

int
CommSvc_Zombify(CommChannel channel,
                int inBH)
{
   return Comm_Zombify(channel, inBH);
}


/* 
                                              
                                       
                                                      
 */

int
CommSvc_IsActive(CommChannel channel)
{
   return Comm_IsActive(channel);
}


/* 
                                                                   
                                                                           
                                                                             
                                                                       
 */

CommTranspInitArgs
CommSvc_GetTranspInitArgs(CommChannel channel)
{
   return Comm_GetTranspInitArgs(channel);
}


/* 
                                                                            
                                               
                                                          
                                        
 */

void *
CommSvc_GetState(CommChannel channel)
{
   return Comm_GetState(channel);
}


/* 
                                                                      
                                           
                                                                         
                                     
                                                                           
                                                                             
                                                                        
                                                                
                                              
                                 
                                                                 
                                                                
                                                   
 */

int
CommSvc_Write(CommChannel channel,
              const CommPacket *packet,
              unsigned long long *timeoutMillis)
{
   return Comm_Write(channel, packet, timeoutMillis);
}


/* 
                                                                               
                                                                             
                                                       
                                                                             
                                                                              
                                                                               
                                                                              
                                                                            
                                                                          
                                                   
                                               
                                
                                        
                                       
                                                                
                                                                                
                                                    
                                                                
                                                  
 */

int
CommSvc_WriteVec(CommChannel channel,
                 const CommPacket *packet,
                 struct kvec **vec,
                 unsigned int *vecLen,
                 unsigned long long *timeoutMillis,
                 unsigned int *iovOffset,
                 int kern)
{
   return Comm_WriteVec(channel, packet, vec, vecLen,
                        timeoutMillis, iovOffset, kern);
}


/* 
                                                                             
                                                                         
                                                                           
                    
                                                           
 */

void
CommSvc_Put(CommChannel channel)
{
   Comm_Put(channel);
}


/* 
                                                                           
                                                                          
                                       
                                                          
 */

void
CommSvc_DispatchUnlock(CommChannel channel)
{
   Comm_DispatchUnlock(channel);
}


/* 
                           
  
                                                                         
                                                                         
                                                                              
                                                                         
                                                                          
                                                        
                                            
 */

int
CommSvc_Lock(CommChannel channel)
{
   return Comm_Lock(channel);
}


/* 
                             
  
                                                                         
                                                                         
                                      
                                                          
 */

void
CommSvc_Unlock(CommChannel channel)
{
   Comm_Unlock(channel);
}


/* 
                                                     
                                                 
                                            
 */

int
CommSvc_ScheduleAIOWork(CommOSWork *work)
{
   return CommOS_ScheduleAIOWork(work);
}


/* 
                                                                         
                                 
                                                                               
 */

unsigned int
CommSvc_RequestInlineEvents(CommChannel channel)
{
   return Comm_RequestInlineEvents(channel);
}


/* 
                                                                             
                                 
                                                                               
 */

unsigned int
CommSvc_ReleaseInlineEvents(CommChannel channel)
{
   return Comm_ReleaseInlineEvents(channel);
}


#if defined(__linux__)
EXPORT_SYMBOL(CommSvc_RegisterImpl);
EXPORT_SYMBOL(CommSvc_UnregisterImpl);
EXPORT_SYMBOL(CommSvc_Alloc);
EXPORT_SYMBOL(CommSvc_Zombify);
EXPORT_SYMBOL(CommSvc_IsActive);
EXPORT_SYMBOL(CommSvc_GetTranspInitArgs);
EXPORT_SYMBOL(CommSvc_GetState);
EXPORT_SYMBOL(CommSvc_Write);
EXPORT_SYMBOL(CommSvc_WriteVec);
EXPORT_SYMBOL(CommSvc_Put);
EXPORT_SYMBOL(CommSvc_DispatchUnlock);
EXPORT_SYMBOL(CommSvc_Lock);
EXPORT_SYMBOL(CommSvc_Unlock);
EXPORT_SYMBOL(CommSvc_ScheduleAIOWork);
EXPORT_SYMBOL(CommSvc_RequestInlineEvents);
EXPORT_SYMBOL(CommSvc_ReleaseInlineEvents);
#endif //                   

