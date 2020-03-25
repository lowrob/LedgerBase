/*
*	sizes.c
*
*	Prints the sizes of Data Structures.
*/


#include <bfs_defs.h>
#include <bfs_recs.h>

main()
{
	printf("Sizes\n\n");
	printf("Pa_rec      : %d\t",sizeof(Pa_rec));
	printf("Ctl_rec     : %d\n",sizeof(Ctl_rec));
	printf("Sch_rec     : %d\t",sizeof(Sch_rec));
	printf("Aud_rec     : %d\n",sizeof(Aud_rec));
	printf("Gl_rec      : %d\t",sizeof(Gl_rec));
	printf("GST_dist    : %d\n",sizeof(GST_dist));
	printf("Re_hdr      : %d\t",sizeof(Re_hdr));
	printf("Re_item     : %d\n",sizeof(Re_item));
	printf("Tr_hdr      : %d\t",sizeof(Tr_hdr));
	printf("Tr_item     : %d\n",sizeof(Tr_item));
	printf("Bd_hdr      : %d\t",sizeof(Bd_hdr));
	printf("Bd_item     : %d\n",sizeof(Bd_item));
	printf("St_mast     : %d\n",sizeof(St_mast));
	printf("St_tran     : %d\n",sizeof(St_tran));
	printf("Alloc_rec   : %d\n",sizeof(Alloc_rec));
	printf("St_sect     : %d\n",sizeof(St_sect));
	printf("Supplier    : %d\n",sizeof(Supplier));
	printf("Po_hdr      : %d\t",sizeof(Po_hdr));
	printf("Po_item     : %d\n",sizeof(Po_item));
	printf("Req_hdr     : %d\t",sizeof(Req_hdr));
	printf("Req_item    : %d\t",sizeof(Req_item));
	printf("Req_reason  : %d\n",sizeof(Req_reason));
	printf("Fa_rec      : %d\n",sizeof(Fa_rec));
	printf("Fa_type     : %d\n",sizeof(Fa_type));
	printf("Fa_dept     : %d\n",sizeof(Fa_dept));
	printf("Fa_transfer : %d\n",sizeof(Fa_transfer));
	printf("Cu_rec      : %d\n",sizeof(Cu_rec));
	printf("Ar_hdr      : %d\t",sizeof(Ar_hdr));
	printf("Ar_item     : %d\n",sizeof(Ar_item));
	printf("Rcpt_hdr    : %d\n",sizeof(Rcpt_hdr));
	printf("Rcpt_item   : %d\n",sizeof(Rcpt_item));
	printf("Invoice     : %d\t",sizeof(Invoice));
	printf("In_hdr      : %d\t",sizeof(In_hdr));
	printf("In_item     : %d\n",sizeof(In_item));
	printf("Chq_rec     : %d\n",sizeof(Chq_rec));
	printf("Chq_hist    : %d\n",sizeof(Chq_hist));
	printf("Reg_rec     : %d\n",sizeof(Reg_rec));
	printf("Ap_hist     : %d\n",sizeof(Ap_hist));
	printf("UP_rec      : %d\n",sizeof(UP_rec));
	printf("Tax_cal     : %d\n",sizeof(Tax_cal));
	exit(0);
}

