
	/*
	*	Get The Parameter Record
	*/
	err = get_param(&pa_rec, BROWSE, 1, e_mesg) ;
	if(err == ERROR) {
		DispError((char *)&s_sth,e_mesg);
		return(ERROR) ;
	}
	else if(err == UNDEF) {
#ifdef ENGLISH
		DispError((char *)&s_sth,"Parameters Are Not Set Up ...");
#else
		DispError((char *)&s_sth,"Parametres ne sont pas etablis... ");
#endif
		return(ERROR) ;
	}
	if(pa_rec.pa_tendering[0] != YES) {
#ifdef ENGLISH
	     DispError((char *)&s_sth,"Tendering System Absent.  See Parameter Maintenance.");
#else
	     DispError((char *)&s_sth,"Systeme Soumission absent. Voir l'entretien des parametres.");
#endif
	     return(ERROR);
	}
