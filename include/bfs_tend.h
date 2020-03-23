/*
*    Source Name : bfs_tend.h
*    System      : Budgetary Financial System.
*
*    Created On  : 7th April 1992.
*
*    Contains Structure/Record Definitions used in this system.
*/

/*
*	Structure/record definition Category file ..
*
*	File Type: ISAM.
*	
*	Primary Key:  categ_num
*/

typedef	struct {
	short	categ_num;		/* Category Number */
	char	categ_desc[41];		/* Category Description */
} Category;

/*
*	Structure/record definition Item Group file ..
*
*	File Type: ISAM.
*
*	Primary Key:  itmgrp_num
*/

typedef	struct {
	short	itmgrp_num;		/* Item Group Number */
	char	itmgrp_desc1[41];	/* Item Group Description */
	char	itmgrp_desc2[41];	/* Item Group Description */
	char	itmgrp_desc3[41];	/* Item Group Description */
} Item_grp;

/*
*	Structure/record definition Catalogue file ..
*
*	File Type: ISAM.
*
*	Primary Key:  cat_num
*/

typedef	struct {
	long	cat_num;		/* Item Group Number */
	char	cat_desc[41];		/* Item Group Description */
	char	cat_uom[7];		/* Unit Of Measure */
	char	cat_awd_supp[11];	/* Awarded Supplier */
	double	cat_min_dollar;		/* Min Dollar Requirement */
	double	cat_unit_price[2];	/* Unit Price 0 - Current year 
					              1 - Previous year */
	double	cat_qty[2];		/* Quantity   0 - Current year 
					              1 - Previous year */
} Catalogue;

/*
*	Structure/record definition Potential Bidder file ..
*
*	File Type: ISAM.
*	
*	Primary Key:  pb_categ_num + pb_supp
*/

typedef	struct {
	short	pb_categ_num;		/* Category Number */
	char	pb_supp[11];		/* Supplier Code */
	long	pb_select;		/* Rotational Selection */
} PotBidder;


/*
*	Structure/record definition Bid file ..
*
*	File Type: ISAM.
*	
*	Primary Key:  bid_supp_cd + bid_cat_num
*	Alt Key-1  :  bid_supp_cd + bid_price + bid_cat_num
*/

typedef	struct {
	char	bid_supp_cd[11];	/* Supplier Code */
	long	bid_cat_num;		/* Catalogue Number */
	short	bid_tend_num;		/* Tender Number */
	double	bid_price;		/* Bid Price */
	char	bid_desc[36];		/* New Description */
	char	bid_status[4];		/* Awarded Status */
} Bid;

/*
*	Structure/record definition Tender History file ..
*
*	File Type: ISAM.
*	
*	Primary Key:  th_cat_num
*/

typedef struct {
	long	th_cat_num;		/* Catalogue Number */
	char	th_supp[3][11];		/* Supplier Code */
	double	th_price[3];		/* Price */
	double	th_qty[3];		/* Quantity */
	double	th_min_dollar[3];	/* Min Dollar */
	long	th_date[3];		/* Date */
} Tend_Hist;
