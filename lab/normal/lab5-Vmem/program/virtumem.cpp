#include <process.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

unsigned simulator(void *);
unsigned inspector(void *);
LPVOID BASE_PTR;
int Actnum = 0;
void job();
int main(int argc, char *argv[]) {
    unsigned ThreadID[2];
    int i = 0;
    _beginthreadex(NULL, 0, simulator, NULL, 0, &ThreadID[0]);
    _beginthreadex(NULL, 0, inspector, NULL, 0, &ThreadID[1]);
    getchar();
	// job();
    return 0;
}
#include <iostream>
#include <string>
void job() {
	std::cout.sync_with_stdio(false);
    MEMORYSTATUS VmemInfo;
    GlobalMemoryStatus(&VmemInfo);
    std::cout << "物理内存总数: " << VmemInfo.dwTotalPhys << " (BYTES)" << std::endl;
    std::cout << "可用物理内存: " << VmemInfo.dwAvailPhys << " (BYTES)" << std::endl;
    std::cout << "页面文件总数: " << VmemInfo.dwTotalPageFile << " (BYTES)" << std::endl;
    std::cout << "可用页面文件数: " << VmemInfo.dwAvailPageFile << " (BYTES)" << std::endl;
    std::cout << "虚存空间总数: " << VmemInfo.dwTotalVirtual << " (BYTES)" << std::endl;
    std::cout << "可用虚存空间数: " << VmemInfo.dwAvailVirtual << " (BYTES)" << std::endl;
    std::cout << "物理存储使用负荷: %" << VmemInfo.dwMemoryLoad << std::endl;
}
unsigned simulator(void *) {
    DWORD OldProtect;
    int randnum;
    printf("Now the simulator procedure has been started.\n");
    srand((unsigned)time(NULL));
	
    randnum = -1;

    while (1) {
        Sleep(500);
        while (Actnum != 0) {  // wait for execution
            Sleep(500);
        }

        randnum = 7 & rand();
        switch (randnum) {
            case 0:
                if (BASE_PTR = VirtualAlloc(NULL, 1024 * 32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) {
                    Actnum = 1;
                }
                break;
            case 1:
                if (VirtualFree(BASE_PTR, 1024 * 32, MEM_DECOMMIT)) {
                    Actnum = 2;
                }
                break;
            case 2:
                if (VirtualFree(BASE_PTR, 0, MEM_RELEASE)) {
                    Actnum = 3;
                }
                break;
            case 3:
                if (VirtualProtect(BASE_PTR, 1024 * 32, PAGE_READONLY, &OldProtect)) {
                    Actnum = 4;
                }
                break;
            case 4:
                if (VirtualLock(BASE_PTR, 1024 * 12)) {
                    Actnum = 5;
                }
                break;
            case 5:
                if (BASE_PTR = VirtualAlloc(NULL, 1024 * 32, MEM_RESERVE, PAGE_READWRITE)) {
                    Actnum = 6;
                }
                break;
            default:
                break;
        }
    }
    return 0;
}
bool validActnum(int v) {
    return v < 7 && v > 0;
}
void PrintState(char *para1, char *tempstr, MEMORY_BASIC_INFORMATION *inspectorinfo1) {
    sprintf(tempstr, "开始地址:0X%x\n", inspectorinfo1->BaseAddress);
    strcat(para1, tempstr);

    sprintf(tempstr, "区块大小:0X%x\n", inspectorinfo1->RegionSize);
    strcat(para1, tempstr);

    sprintf(tempstr, "目前状态:0X%x\n", inspectorinfo1->State);
    strcat(para1, tempstr);

    sprintf(tempstr, "分配时访问保护:0X%x\n", inspectorinfo1->AllocationProtect);
    strcat(para1, tempstr);

    sprintf(tempstr, "当前访问保护:0X%x\n", inspectorinfo1->Protect);
    strcat(para1, tempstr);

    strcat(para1, "(状态:10000代表未分配；1000代表提交；2000代表保留；)\n");
    strcat(para1, "(保护方式：0代表其它；1代表禁止访问；2代表只读；4代表读写;\n10代表可执");
    strcat(para1, "行;20代表可读和执行;40代表可读写和执行);\n********************************\n");
}
void PrintMemory(char *para1, char *tempstr) {
    MEMORYSTATUS Vmeminfo;
    GlobalMemoryStatus(&Vmeminfo);
    strcat(para1, "当前整体存储统计如下\n");
    sprintf(tempstr, "物理内存总数：%d(BYTES)\n", Vmeminfo.dwTotalPhys);
    strcat(para1, tempstr);
    sprintf(tempstr, "可用物理内存：%d(BYTES)\n", Vmeminfo.dwAvailPhys);
    strcat(para1, tempstr);
    sprintf(tempstr, "页面文件总数：%d(BYTES)\n", Vmeminfo.dwTotalPageFile);
    strcat(para1, tempstr);
    sprintf(tempstr, "可用页面文件数：%d(BYTES)\n", Vmeminfo.dwAvailPageFile);
    strcat(para1, tempstr);
    sprintf(tempstr, "虚存空间总数：%d(BYTES)\n", Vmeminfo.dwTotalVirtual);
    strcat(para1, tempstr);
    sprintf(tempstr, "可用虚存空间数：%d(BYTES)\n", Vmeminfo.dwAvailVirtual);
    strcat(para1, tempstr);
    sprintf(tempstr, "物理存储使用负荷：%%%d\n\n\n\n", Vmeminfo.dwMemoryLoad);
    strcat(para1, tempstr);
    printf("%s", para1);
}
unsigned inspector(void *) {
    char para1[3000];
    char tempstr[100];

    MEMORY_BASIC_INFORMATION inspectorinfo1;

    int QuOut = 0;
    int structsize = sizeof(MEMORY_BASIC_INFORMATION);

    printf("Hi,  now inspector begin to work\n");

    while (1) {
        Sleep(1000);
        if (Actnum != 0) {
            if (validActnum(Actnum)) {
                memset(&inspectorinfo1, 0, structsize);
                VirtualQuery((LPVOID)BASE_PTR, &inspectorinfo1, structsize);
                switch (Actnum) {
                    case 1:
                        strcpy(para1, "目前执行动作：虚存的保留与提交\n");
                        break;
                    case 2:
                        strcpy(para1, "目前执行动作：虚存的除配\n");
                        break;
                    case 3:
                        strcpy(para1, "目前执行动作：虚存的除配并释放虚存空间\n");
                        break;
                    case 4:
                        strcpy(para1, "目前执行动作：改变虚存内存页的保护\n");
                        break;
                    case 5:
                        strcpy(para1, "目前执行动作：锁定虚存内存页\n");
                        break;
                    case 6:
                        strcpy(para1, "目前执行动作：虚存的保留\n");
                        break;
                }
            }
            PrintState(para1, tempstr, &inspectorinfo1);
            PrintMemory(para1, tempstr);
            Actnum = 0;
            Sleep(500);
        }
    }
    return 0;
}