#include "main.h"

char* homePath = NULL;

void procLpMsg(const char* logicName, const int fd, const char* recvbuf, const int recvlen)
{
   char sendbuf[1024] = {0};
   int sendlen = 0;
   procLgModule(recvbuf, recvlen, sendbuf, &sendlen);
   commSend(fd, sendbuf, &sendlen);
}

int main(int argc, char** argv)
{
   int ret,i;
   char tmp[256] = {0};
   struct timeval curTimeval;
   homePath = getenv("DUSTHOME");
   if(NULL == homePath) {
      ERRORTIP("$DUSTHOME is null, try export...");
   }

   //init logModule
   memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
   sprintf(tmp, "%s/conf/dust.conf", homePath);
   ret = logInit(tmp);
   if(ret) {
      ERRORTIP("logInit %s", moduleErrInfo(log, ret));
   }
   log(LOG_INFO, "App<%s> start running!", moduleVersion(app));

   //check running environment
   if(NULL == moduleVersion(app)) {
      log(LOG_WARNING, "it seems that appVerInfo is not set");
   }
   if(NULL == homePath) {
      log(LOG_ERROR, "DUSTHOME is not set");
      exit(1);
   }
   if(strlen(homePath) > 200) {
      log(LOG_ERROR, "$DUSTHOME is too long");
      exit(1);
   }
   const char* subdir[] = {"", "bin", "conf", "log", "data"};
   for(i=0; i<sizeof(subdir)/sizeof(subdir[0]); i++) {
      memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
      sprintf(tmp, "%s/%s", homePath, subdir[i]);
      if(access(tmp, F_OK)) {
         log(LOG_ERROR, "%s does not exist, try mkdir %s", tmp);
         exit(1);
      }
   }
   
   //init commModule
   memset(tmp, 0, sizeof(tmp)/sizeof(tmp[0]));
   sprintf(tmp, "%s/conf/dust.conf", homePath);
   ret = commInit(tmp);
   if(ret) {
      log(LOG_ERROR, "commInit %s", moduleErrInfo(comm, ret));
      exit(1);
   }
   ret = commConnect(NULL);
   if(ret) {
      log(LOG_ERROR, "commConnect %s", moduleErrInfo(comm, ret));
      exit(1);
   }

   //init lgModule
   initLgModule("*** HUANYING WO DE QINQINBAO ***\n  Enter 'help' to get help msg~\n-----------------------------------");
   addDefaultCmdFunction();
   ret = commSetFunc("dustLgModule", (RegisterFunc*)&welcomeLp, &procLpMsg);
   if(ret) {
      log(LOG_ERROR, "it seems that logicName<dustLgModule> is not set");
      exit(1);
   }
 
   //init every function
   initAll();

   //step into endless loop
   while(1) {
      gettimeofday(&curTimeval, NULL);
      checkEvent(curTimeval);
      commProcess();
   }

   return 0;
}

