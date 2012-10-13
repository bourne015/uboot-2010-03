#include <common.h>

#ifdef CONFIG_S3C64XX
#include <asm/io.h>
#include <linux/mtd/nand.h>
#include <asm/arch/s3c6400.h>

static int nandll_read_page (uchar *buf, ulong addr, int large_block)
{
        int i;
	int page_size = 512;

	if (large_block)
		page_size = 2048;


	/*使能芯片*/
        NAND_ENABLE_CE();

        NFCMD_REG = NAND_CMD_READ0;

        /* Write Address */
        NFADDR_REG = 0;

	if (large_block)
	        NFADDR_REG = 0;

	NFADDR_REG = (addr) & 0xff;
	NFADDR_REG = (addr >> 8) & 0xff;
	NFADDR_REG = (addr >> 16) & 0xff;

	if (large_block)
		NFCMD_REG = NAND_CMD_READSTART;

        NF_TRANSRnB();

	/* for compatibility(2460). u32 cannot be used. by scsuh */
	for(i=0; i < page_size; i++) {
                *buf++ = NFDATA8_REG;
        }

        NAND_DISABLE_CE();
        return 0;
}

static int nandll_read_blocks (ulong dst_addr, ulong size, int large_block)
{
        uchar *buf = (uchar *)dst_addr;
        int i;
	uint page_shift = 9;//512

	if (large_block)
		page_shift = 11;//2048

        /* Read pages */
        for (i = 0; i < (0x3c000>>page_shift); i++, buf+=(1<<page_shift)) {
                nandll_read_page(buf, i, large_block);
        }

        return 0;
}

int copy_uboot_to_ram (void)
{
	int large_block = 0;
	int i;
	vu_char id;
	
	
	/*使能芯片*/
        NAND_ENABLE_CE();
        /*读取ID*/
        NFCMD_REG = NAND_CMD_READID;
        NFADDR_REG =  0x00;

	/* 等待一会 */
        for (i=0; i<200; i++);
	id = NFDATA8_REG;
	id = NFDATA8_REG;


	/*id>0x80则为大页*/
	if (id > 0x80)
		large_block = 1;

	 /*CONFIG_SYS_PHY_UBOOT_BASE拷贝地址，0x3c000为大小240K*/
	return nandll_read_blocks(CONFIG_SYS_PHY_UBOOT_BASE, 0x3c000, large_block);
}

#endif
