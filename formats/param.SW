	char	pa_co_name[51] ;	/* Company Name */
	short	pa_distccno;		/* District Cost Center Number */
	short	pa_wareccno;		/* Warehouse Cost Center Number */
	char	pa_co_or_dist[1] ;	/* Company or School district. C or D */
	char	pa_aps[1] ;		/* A/P system Present? Y or N */
	char	pa_requisition[1] ;	/* Requisition system Present? Y or N */
	char	pa_pos[1] ;		/* Cancel pending POs at year end.*/
	char	pa_poinv[1] ;		/* PO and Inventory Interface */
	char	pa_stores[1] ;		/* Inventrory system Present? Y or N */
	char	pa_fa[1] ;		/* Fixed Assests system Present? */
	char	pa_ars[1] ;		/* A/R system Present? Y or N */
	char	pa_funds[1] ;		/* More than 1 Fund(Control Record).*/
	char	pa_glmast[1] ;		/* G/L Master File Present. Y or N */
	char	pa_cheque[1] ;		/* Consolidated Cheques. Y or N. This*/
	short	pa_cur_period;		/* Current Period 1 to 13 */
	short	pa_open_per ;		/* No of open periods to post journal*/
	long	pa_date ;		/* Today's System Date */
	short	pa_no_periods;		/* Number of periods is 12 or 13 */
	short	pa_bdgt_key1;		/* Budget Report key 1 */
	short	pa_bdgt_key2;		/* Budget Report key 2 */
	short	pa_bdgt_key3;		/* Budget Report key 3 */
	short	pa_bdgt_key4;		/* Budget Report key 4 */
	short	pa_bdgt_key5;		/* Budget Report key 5 */
	short	pa_bdgt_key6;		/* Budget Report key 6 */
	short	pa_bdgt_key7;		/* Budget Report key 7 */
	short	pa_cc_key;		/* Cost Center key */
	short	pa_due_days_po;		/* Number of days before due date (PO)*/
	short	pa_due_days_ap;		/* Number of days before due date (AP)*/
	short	pa_due_days_ar;		/* Number of days before due date (AR)*/
	char	pa_budget[1];		/* Transfer budget to new year YorN */
	char	pa_dist_gst[1];		/* Distribute GST over accts. YorN */
	char	pa_regist[11];		/* GST Registration number */
	char	pa_gst_tax[2];		/* GST Tax Field Default */
	char	pa_pst_tax[2];		/* PST Tax Field Default */
	double	pa_purlimit;		/* Purchasing limit */
