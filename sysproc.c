#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_date(void)
{
  struct rtcdate* date;
  if(argptr(0, (void *)&date, sizeof(*date) < 0)) {
    return -1;
  }
  cmostime(date);
  return 0;
}

int 
sys_ps(void) {
  struct proc* p;
  // cprintf("Get in ps\n");
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if (p->pid == 0)
      continue;
    cprintf("pid:%d, level %d, state:%d\n", p->pid, p->current_level, p->state);
  }
  return 0;
}

int 
sys_su(void) {
  // get my proc
  // struct proc* p = myproc();
  char *usr_name, *passwd;
  if(argptr(0, &usr_name, 16) < 0) {
    return -1;
  }
  if(argptr(1, &passwd, 16) < 0) {
    return -1;
  }
  for (int i = 0; i < MAX_USERINFO_LENGTH; i++) {
    if (strncmp(system_users[i].username, usr_name, strlen(system_users[i].username)) == 0) {
      if(strncmp(system_users[i].password, passwd, strlen(system_users[i].password)) == 0) {
        current_user = system_users + i;
        cprintf("change to user %s\n", current_user->username);
        return 0;
      } else {
        cprintf("password error!");
        return -1;
      }
    }
  }
  cprintf("cannot find user %s!\n", usr_name);
  return -1;
}

int
sys_dproc(void)
{
  struct proc* p;
  int cur_level;
  // char *p;argptr(1, &p, n) < 0
  if(argptr(0, (char**)&p, sizeof(struct proc)) < 0)
    return -1;
  if(argint(1, &cur_level) < 0)
    return -1;
  return proc_down(p, cur_level);
}

int 
sys_logname(void) {
  // struct proc* p = myproc();
  cprintf("%s", current_user->username);
  return 0;
}