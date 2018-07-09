/* Program to access APIC - Intel 82093 */ 
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>

/* This device is Memory Mapped */

/* 
   We do not access the device registers directly.
   We remap the range of addresses from starting address into the Kernel Space
   and access the device using those addresses.
*/

/* Value of the base register from DataSheet */
#define IOAPIC_BASE 0xFEC00000

void *io;

/* Entry Function */
int init_module(void)
{
  void *ioregsel = NULL, *iowin = NULL;
  uint32_t id, version, maxirq, regLow, regHigh, ii, jj;

  /* These are offset values meant to be passed as parameters to IOREGSEL */
  uint32_t redirectionTable[] = 
  {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
    0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
  };

  /* 
  ioremap - remaps a physical address range into the processor's virtual address 
            space (kernel's linear address), making it available to the kernel
            IOAPIC_BASE: Physical address of IOAPIC
            SIZE: size of the resource to map 
  */
  io = ioremap(IOAPIC_BASE, PAGE_SIZE);

  /* As per IOAPIC Datasheet 0x00 is I/O REGISTER SELECT of size 32 bits */
  ioregsel = (io + 0x00);

  /* As per IOAPIC Datasheet 0x10 is I/O WINDOW REGISTER of size 32 bits */ 
  iowin    = (io + 0x10);

  printk("%s\n",__func__);
  printk("IO APIC\t\t");

  /* 
    Read IOAPIC IDENTIFICATION 
    As per IOAPIC Datasheet IOAPIC IDENTIFICATION REGISTER 
    Address Offset : 0x00 

    IOAPIC IDENTIFICATION REGISTER

    Bits                 Description
    ********************************
    31:28                Reserved
 
    27:24                This 4 bit field contains the IOAPIC 
                         identification.      

    23:0                 Reserved        
  */
  iowrite32(0x00, ioregsel);
  id = (ioread32(iowin) >> 24) & 0x0F;
  printk("ID = 0x%02X\t\t",id);

  /* 
    Read IOAPIC VERSION
    As per IOAPIC Datasheet IOAPIC VERSION REGISTER 
    Address Offset : 0x01 

    IOAPIC VERSION REGISTER

    Bits                 Description
    ********************************
    31:24                Reserved        

    23:16                This field contains number of interrupt
                         input pins for the IOAPIC minus one.

    15:8                 Reserved        

    7:0                  This 8 bit field identifies the implementation 
                         version.
  */
  iowrite32(0x01, ioregsel);
  version = ioread32(iowin) & 0xFF;
  printk("Version = 0x%02X\n",version);

  /* Mask Rest and Access Bit 16-23 */
  maxirq = (ioread32(iowin) >> 16) & 0x00FF;
  maxirq = maxirq + 1;
  printk("No of Redirection Entries (MAX IRQ) =  0x%02X\n",maxirq);

  /* Each Interrupt is associated with two 32 bit registers (High and Low) */
  /* Each high and low register has an associated redirection table offset */
  for(ii = 0, jj = 0; ii < maxirq; ii++,jj++)
  {
    iowrite32(redirectionTable[jj], ioregsel);
    regLow  = ioread32(iowin);
    jj++;
    iowrite32(redirectionTable[jj], ioregsel);
    regHigh = ioread32(iowin);
    
    if((ii % 3) == 0)
      printk("\n");
    printk("\t0x%02X : ",ii);
    printk("%08X%08X",regHigh, regLow);
  }
  printk("\r\n");
  return 0;
}

/* Exit Function */
void cleanup_module(void)
{
  printk("%s\n",__func__);
  /* Unmap the allocated address range in kernel */ 
  iounmap(io);
}

/* Comments which are retained in code */
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_DESCRIPTION("Access APIC");
MODULE_LICENSE("GPL");
