
%typemap(out) int, size_t
%{ Scm_MakeInteger($1) %}

%typemap(out) bool
%{ $1 ? SCM_TRUE : SCM_FALSE %}

%typemap(out) string
%{ Scm_MakeString($1.c_str(), $1.length, $1.length, 0) %}
