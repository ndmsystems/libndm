#include <string.h>
#include <ndm/strarg.h>
#include <ndm/strvec.h>
#include "test.h"

int main()
{
	struct ndm_strvec_t v = NDM_STRVEC_INIT;

	NDM_TEST(ndm_strarg_parse("", &v));
	NDM_TEST(ndm_strvec_is_empty(&v));

	NDM_TEST(ndm_strarg_parse("test", &v));
	NDM_TEST(!ndm_strvec_is_empty(&v));
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "test") == 0);

	NDM_TEST(ndm_strarg_parse("   test   arg1   ", &v));
	NDM_TEST(ndm_strvec_size(&v) == 2);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "test") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 1), "arg1") == 0);

	NDM_TEST(ndm_strarg_parse("test\targ1", &v));
	NDM_TEST(ndm_strvec_size(&v) == 2);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "test") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 1), "arg1") == 0);

	NDM_TEST(ndm_strarg_parse("test\t'long arg1 ' ", &v));
	NDM_TEST(ndm_strvec_size(&v) == 2);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "test") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 1), "long arg1 ") == 0);

	NDM_TEST(ndm_strarg_parse("test\t'long \"arg1\" ' ", &v));
	NDM_TEST(ndm_strvec_size(&v) == 2);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "test") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 1), "long \"arg1\" ") == 0);

	NDM_TEST(ndm_strarg_parse("t\t'long \"arg1\" ' \"arg2 'long' \" ", &v));
	NDM_TEST(ndm_strvec_size(&v) == 3);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 0), "t") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 1), "long \"arg1\" ") == 0);
	NDM_TEST(strcmp(ndm_strvec_at(&v, 2), "arg2 'long' ") == 0);

	NDM_TEST(!ndm_strarg_parse("t\t'long \"arg1\" ", &v));
	NDM_TEST(!ndm_strarg_parse("t\t\"long 'arg1' ", &v));

	ndm_strvec_clear(&v);

	return NDM_TEST_RESULT;
}
