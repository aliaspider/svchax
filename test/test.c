#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "../svchax.h"

static const char* test_result;

s32 set_test_result(void)
{
   __asm__ volatile("cpsid aif \n\t");
   test_result = "SUCCESS !!!";
   return 0;
}

static void test_thread_entry(void)
{
   svcBackdoor(set_test_result);
   svcExitThread();
}
int main(int argc, char** argv)
{
   gfxInit(GSP_BGR8_OES, GSP_RGB565_OES, false);
   gfxSet3D(false);
   consoleInit(GFX_BOTTOM, NULL);
   printf("svchax\n");
   printf("Press Start to exit.\n");
   printf("kernel version : %i.%i.%i\n\n", (int)*(u8*)(0x1FF80003), (int)*(u8*)(0x1FF80002), (int)*(u8*)(0x1FF80001));

   osSetSpeedupEnable(false);
   svchax_init(true);
   osSetSpeedupEnable(true);

   if(__ctr_svchax)
   {
      Handle test_thread;
      static u8 test_thread_stack[0x1000];

      test_result ="FAILED !!!";
      svcBackdoor(set_test_result);
      printf("main thread ACL patched : %s\n", test_result);

      test_result ="FAILED !!!";
      svcCreateThread(&test_thread, (ThreadFunc)test_thread_entry, 0, (u32*)&test_thread_stack[0x1000], 0x18, 0);
      svcWaitSynchronization(test_thread, U64_MAX);
      svcCloseHandle(test_thread);
      printf("process ACL patched     : %s\n", test_result);
   }

   if(__ctr_svchax_srv)
   {
      Handle tmp;
      printf("service access patched  : ");
      if (srvGetServiceHandle(&tmp, "am:net") >= 0)
      {
         svcCloseHandle(tmp);
         printf("SUCCESS !!!\n");
      }
      else
         printf("FAILED !!!\n");
   }



   u32 frames = 0;

   printf("\n");
   while (aptMainLoop())
   {
      hidScanInput();
      u32 kDown = hidKeysDown();

      if (kDown & KEY_START)
         break;

      printf("frames : %u\r", (unsigned)frames++);
      gspWaitForVBlank();
      fflush(stdout);

   }

   gfxExit();
   return 0;
}





