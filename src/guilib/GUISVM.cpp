#include "guilib/GUISVM.h"
#include "gui/GUI.h"
#include "lib/io.h"

CGUISVM::CGUISVM(CGUI * gui_)
  : gui(gui_)
{
	svm=NULL;
}

CGUISVM::~CGUISVM()
{
}

bool CGUISVM::new_svm(char* param)
{
  param=CIO::skip_spaces(param);
  
  if (strcmp(param,"LIGHT")==0)
    {
      delete svm;
      svm= new CSVMLight();
      CIO::message("created SVMLight object\n") ;
    }
  else if (strcmp(param,"CPLEX")==0)
    {
#ifdef SVMCPLEX
      delete svm;
      svm= new CSVMCplex();
      CIO::message("created SVMCplex object\n") ;
#else
      CIO::message("CPLEX SVM disabled\n") ;
#endif
    }
  else if (strcmp(param,"MPI")==0)
    {
#ifdef SVMMPI
#if  defined(HAVE_MPI) && !defined(DISABLE_MPI)
      delete svm;
      svm= new CSVMMPI();
      CIO::message("created SVMMPI object\n") ;
#endif
#else
      CIO::message("MPI SVM disabled\n") ;
#endif
    }
  else
    return false;
  
  return (svm!=NULL);
}

bool CGUISVM::train(char* param)
{
	param=CIO::skip_spaces(param);

	CFeatures* features=gui->guifeatures.get_train_features();
	CPreProc * preproc=gui->guipreproc.get_preproc() ;

	if (!svm)
	{
		CIO::message("no svm available") ;
		return false ;
	}

	if (!features)
	{
		CIO::message("no training features available") ;
		return false ;
	}

	if (preproc)
	{
		CIO::message("using preprocessor: %s\n", preproc->get_name());
		if (features->get_feature_type()!=preproc->get_feature_type())
		{
			CIO::message("preprocessor does not fit to features");
			return false;
		}


		CIO::message("S:initializing train features %ldx%ld\n", ((CRealFeatures*) features)->get_number_of_examples(), ((CRealFeatures*) features)->get_num_features());
		preproc->init(features);
		CIO::message("E:initializing train features %ldx%ld\n", ((CRealFeatures*) features)->get_number_of_examples(), ((CRealFeatures*) features)->get_num_features());
	}
	else
		CIO::message("doing without preproc\n");
	
	features->set_preproc(preproc);
	((CRealFeatures*) features)->preproc_feature_matrix();
	CIO::message("I:train features %ldx%ld\n", ((CRealFeatures*) features)->get_number_of_examples(), ((CRealFeatures*) features)->get_num_features());
	
	//  if (!svm->check_feature_type(f))
	//    {
	//      CIO::message("features do not fit to svm") ;
	//      return false ;
	//    }

	CIO::message("starting svm training\n") ;
	svm->set_C(C) ;
	svm->set_kernel(gui->guikernel.get_kernel()) ;
	return svm->svm_train(features) ;
}

bool CGUISVM::test(char* param)
{
	param=CIO::skip_spaces(param);
	char outputname[1024];
	char rocfname[1024];
	FILE* outputfile=stdout;
	FILE* rocfile=NULL;
	int numargs=-1;

	numargs=sscanf(param, "%s %s", outputname, rocfname);

	if (numargs>=1)
	{
		outputfile=fopen(outputname, "w");

		if (!outputfile)
		{
			CIO::message(stderr,"ERROR: could not open %s\n",outputname);
			return false;
		}

		if (numargs==2) 
		{
			rocfile=fopen(rocfname, "w");

			if (!rocfile)
			{
				CIO::message(stderr,"ERROR: could not open %s\n",rocfname);
				return false;
			}
		}
	}

	CFeatures* trainfeatures=gui->guifeatures.get_train_features();
	CFeatures* testfeatures=gui->guifeatures.get_test_features();
	CPreProc * preproc=gui->guipreproc.get_preproc();
	CIO::message("I:train features %ldx%ld\n", ((CRealFeatures*) trainfeatures)->get_number_of_examples(), ((CRealFeatures*) trainfeatures)->get_num_features());
	CIO::message("I:test features %ldx%ld\n", ((CRealFeatures*) testfeatures)->get_number_of_examples(), ((CRealFeatures*) testfeatures)->get_num_features());

	if (!svm)
	{
		CIO::message("no svm available") ;
		return false ;
	}
	if (!trainfeatures)
	{
		CIO::message("no training features available") ;
		return false ;
	}

	if (!testfeatures)
	{
		CIO::message("no test features available") ;
		return false ;
	}

	if (preproc)
	{
		CIO::message("using preprocessor: %s\n", preproc->get_name());
		if (trainfeatures->get_feature_type()!=preproc->get_feature_type() || testfeatures->get_feature_type()!=preproc->get_feature_type())
		{
			CIO::message("preprocessor does not fit to features");
			return false;
		}
		CIO::message("S:initializing train features %ldx%ld\n", ((CRealFeatures*) trainfeatures)->get_number_of_examples(), ((CRealFeatures*) trainfeatures)->get_num_features());
		preproc->init(trainfeatures);
		CIO::message("E:initializing train features %ldx%ld\n", ((CRealFeatures*) trainfeatures)->get_number_of_examples(), ((CRealFeatures*) trainfeatures)->get_num_features());
	}
	else
		CIO::message("doing without preproc\n");
	

	trainfeatures->set_preproc(preproc);
	trainfeatures->preproc_feature_matrix();
	CIO::message("I:train features %ldx%ld\n", ((CRealFeatures*) trainfeatures)->get_number_of_examples(), ((CRealFeatures*) trainfeatures)->get_num_features());
	CIO::message("I:test features %ldx%ld\n", ((CRealFeatures*) testfeatures)->get_number_of_examples(), ((CRealFeatures*) testfeatures)->get_num_features());
	
	testfeatures->set_preproc(preproc);
	testfeatures->preproc_feature_matrix();
	CIO::message("I:train features %ldx%ld\n", ((CRealFeatures*) trainfeatures)->get_number_of_examples(), ((CRealFeatures*) trainfeatures)->get_num_features());
	CIO::message("I:test features %ldx%ld\n", ((CRealFeatures*) testfeatures)->get_number_of_examples(), ((CRealFeatures*) testfeatures)->get_num_features());

	//  if (!svm->check_feature_type(f))
	//    {
	//      CIO::message("features do not fit to svm") ;
	//      return false ;
	//    }

	CIO::message("starting svm testing\n") ;
	svm->set_C(C) ;
	svm->set_kernel(gui->guikernel.get_kernel()) ;
	REAL* output= svm->svm_test(testfeatures, trainfeatures) ;
	
	int total=testfeatures->get_number_of_examples();

	int* label= new int[total];	

	REAL* fp= new REAL[total];	
	REAL* tp= new REAL[total];	

	for (int dim=0; dim<total; dim++)
	{
		label[dim]= testfeatures->get_label(dim);

		if (math.sign((REAL) output[dim])==label[dim])
			fprintf(outputfile,"%+.8g (%+d)\n",(double) output[dim], label[dim]);
		else
			fprintf(outputfile,"%+.8g (%+d)(*)\n",(double) output[dim], label[dim]);
	}

	int possize,negsize;
	int pointeven=math.calcroc(fp, tp, output, label, total, possize, negsize, rocfile);

	double correct=possize*tp[pointeven]+(1-fp[pointeven])*negsize;
	double fpo=fp[pointeven]*negsize;
	double fne=(1-tp[pointeven])*possize;

	printf("classified:\n");
	printf("\tcorrect:%i\n", int (correct));
	printf("\twrong:%i (fp:%i,fn:%i)\n", int(fpo+fne), int (fpo), int (fne));
	printf("of %i samples (c:%f,w:%f,fp:%f,tp:%f)\n",total, correct/total, 1-correct/total, (double) fp[pointeven], (double) tp[pointeven]);

	delete[] fp;
	delete[] tp;
	delete[] output;
	delete[] label;
	return true;
}

bool CGUISVM::set_kernel()
{
  CIO::not_implemented() ;
  return false ;
}

bool CGUISVM::get_kernel()
{
  CIO::not_implemented() ;
  return false ;
}

bool CGUISVM::set_preproc()
{
  CIO::not_implemented() ;
  return false ;
}

bool CGUISVM::get_preproc()
{
  CIO::not_implemented() ;
  return false ;
}

bool CGUISVM::load(char* param)
{
    bool result=false;
    param=CIO::skip_spaces(param);
    char filename[1024];
    char type[1024];

    if ((sscanf(param, "%s %s", filename, type))==2)
    {

	new_svm(type);

	FILE* model_file=fopen(filename, "r");

	if (model_file)
	{
	    if (svm && svm->load(model_file))
	    {
		printf("file successfully read\n");
		result=true;
	    }
	    else
		CIO::message("svm creation/loading failed\n");

	    fclose(model_file);
	}
	else
	    CIO::message("opening file %s failed\n", filename);

	return result;
	CIO::not_implemented() ;
	return false ;
    }
    else
	CIO::message("see help for parameters\n");
}

bool CGUISVM::save(char* param)
{
    bool result=false;
    param=CIO::skip_spaces(param);

    if (svm)
    {
	FILE* file=fopen(param, "w");

	if ((!file) ||	(!svm->save(file)))
	    printf("writing to file %s failed!\n", param);
	else
	{
	    printf("successfully written svm into \"%s\" !\n", param);
	    result=true;
	}

	if (file)
	    fclose(file);
    }
    else
	CIO::message("create svm first\n");

    return result;
    CIO::not_implemented() ;
    return false ;
}

bool CGUISVM::set_C(char* param)
{
	param=CIO::skip_spaces(param);

	sscanf(param, "%le", &C) ;
	CIO::message("Set to C=%f\n", C) ;
	return true ;  
}
