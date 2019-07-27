#include "swigmod.h"

class GAUCHE : public Language {
protected:
  /* General DOH objects used for holding the strings */
  File *f_begin;
  File *f_runtime;
  File *f_header;
  File *f_wrappers;
  File *f_init;

public:

  virtual void main(int argc, char *argv[]) {
    printf("I'm the Gauche module.\n");

    /* Set language-specific subdirectory in SWIG library */
    SWIG_library_directory("gauche");

    /* Set language-specific preprocessing symbol */
    Preprocessor_define("SWIGGAUCHE 1", 0);

    /* Set language-specific configuration file */
    SWIG_config_file("gauche.swg");

    /* Set typemap language (historical) */
    SWIG_typemap_lang("gauche");

  }

  virtual int top(Node *n);
  virtual int functionWrapper(Node *n);
  
};

extern "C" Language *
swig_gauche(void) {
  return new GAUCHE();
}

int GAUCHE::top(Node *n) {
  printf("Generating code.\n");

  /* Get the module name */
  String *module = Getattr(n, "name");

  /* Get the output file name */
  String *outfile = Getattr(n, "outfile");

  /* Initialize I/O (see next section) */
  f_begin = NewFile(outfile, "w", SWIG_output_files());
  if (!f_begin) {
    FileErrorDisplay(outfile);
    SWIG_exit(EXIT_FAILURE);
  }
  f_runtime = NewString("");
  f_init = NewString("");
  f_header = NewString("");
  f_wrappers = NewString("");

  /* Register file targets with the SWIG file handler */
  Swig_register_filebyname("begin", f_begin);
  Swig_register_filebyname("header", f_header);
  Swig_register_filebyname("wrapper", f_wrappers);
  Swig_register_filebyname("runtime", f_runtime);
  Swig_register_filebyname("init", f_init);

  /* Output module initialization code */
  Swig_banner(f_begin);

  /* Emit code for children */
  Language::top(n);

  /* Write all to the file */
  Dump(f_runtime, f_begin);
  Dump(f_header, f_begin);
  Dump(f_wrappers, f_begin);
  Wrapper_pretty_print(f_init, f_begin);

  /* Cleanup files */
  Delete(f_runtime);
  Delete(f_header);
  Delete(f_wrappers);
  Delete(f_init);
  Delete(f_begin);

  return SWIG_OK;
}

int GAUCHE::functionWrapper(Node *n) {
  /* Get some useful attributes of this function */
  String   *name   = Getattr(n, "sym:name");
  SwigType *type   = Getattr(n, "type");
  ParmList *parms  = Getattr(n, "parms");
  String   *parmstr= ParmList_str_defaultargs(parms); // to string
  String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
  String   *action = Getattr(n, "wrap:action");
  String   *tm;

  // Printf(f_wrappers, "functionWrapper   : %s\n", func);
  // Printf(f_wrappers, "           action : %s\n", action);

  Wrapper *wrapper = NewWrapper();
  String *wname = Swig_name_wrapper(name);

  emit_parameter_variables(parms, wrapper);
  emit_return_variable(n, type, wrapper);

  Printv(wrapper->def, "ScmObj ", wname, "(ScmObj *args, int argc, void *data) {", NIL);

  if ((tm = Swig_typemap_lookup_out("out", n, Swig_cresult_name(), wrapper, action))) {
    Replaceall(tm, "$source", Swig_cresult_name());
    if (GetFlag(n, "feature:new")) {
      Replaceall(tm, "$owner", "1");
    } else {
      Replaceall(tm, "$owner", "0");
    }
    Printf(wrapper->code, "\nreturn %s\n", tm, NIL);
  } else {
    Printf(wrapper->code, "return SCM_NIL\n", NIL);
  }

  Printv(wrapper->code, "}", NIL);

  Wrapper_print(wrapper, f_wrappers);

  Delete(wname);
  DelWrapper(wrapper);

  return SWIG_OK;
}

// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
