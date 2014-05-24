#include <ctype.h>
#include <stddef.h>
#include <ndm/net.h>
#include "test.h"

int main()
{
	size_t i;
	char test_domain[] = "a~a.com";

	for (i = 1; i < 256; ++i) {
		test_domain[1] = (char) i;

		NDM_TEST(
			((isalnum((int) i) || i == '-' || i == '.') &&
			 ndm_net_is_domain_name(test_domain)) ||
			(!(isalnum((int) i) || i == '-' || i == '.') &&
			 !ndm_net_is_domain_name(test_domain)));
	}

	NDM_TEST(!ndm_net_is_domain_name(""));
	NDM_TEST(!ndm_net_is_domain_name(" "));
	NDM_TEST(!ndm_net_is_domain_name("     "));
	NDM_TEST(!ndm_net_is_domain_name("domain.ru."));
	NDM_TEST(!ndm_net_is_domain_name("domain-.ru"));
	NDM_TEST(!ndm_net_is_domain_name("domain--.ru"));
	NDM_TEST(!ndm_net_is_domain_name("-domain.ru"));
	NDM_TEST(!ndm_net_is_domain_name("-domain..ru"));
	NDM_TEST(!ndm_net_is_domain_name("domain.-.ru"));
	NDM_TEST(!ndm_net_is_domain_name("domain.test-.ru"));
	NDM_TEST(!ndm_net_is_domain_name(".domain.ru"));
	NDM_TEST(!ndm_net_is_domain_name("-.domain.ru"));
	NDM_TEST(!ndm_net_is_domain_name("-.domain-a.ru"));
	NDM_TEST(!ndm_net_is_domain_name("t-.domain-a.ru"));
	NDM_TEST(!ndm_net_is_domain_name("-.ni"));
	NDM_TEST(!ndm_net_is_domain_name(
		"very-long-long-long-long-long"
		"long-long-long-long-long-domainname.005.com"));
	NDM_TEST(!ndm_net_is_domain_name("domain-0.ru  "));
	NDM_TEST(!ndm_net_is_domain_name("  a0-domain.ru"));
	NDM_TEST(!ndm_net_is_domain_name(" 005.com  "));
	NDM_TEST(!ndm_net_is_domain_name("   ru.shortn   "));

	NDM_TEST(ndm_net_is_domain_name("domain"));
	NDM_TEST(ndm_net_is_domain_name("a.a"));
	NDM_TEST(ndm_net_is_domain_name(
		"not-so-long-long-long-long-long-"
		"long-long-long-long-domain-name.005.com"));
	NDM_TEST(ndm_net_is_domain_name("ru.veryverylonglonglonglonglongname"));
	NDM_TEST(ndm_net_is_domain_name("ru.longna"));
	NDM_TEST(ndm_net_is_domain_name("test-d.domain-a.ru"));
	NDM_TEST(ndm_net_is_domain_name("te--st------d.do----main-a.ru"));
	NDM_TEST(ndm_net_is_domain_name("domain-0.ru"));
	NDM_TEST(ndm_net_is_domain_name("a0-domain.ru"));
	NDM_TEST(ndm_net_is_domain_name("005.com"));
	NDM_TEST(ndm_net_is_domain_name("ru.shortn"));
	NDM_TEST(ndm_net_is_domain_name("ru.ru"));
	NDM_TEST(ndm_net_is_domain_name("a.ni"));

	return NDM_TEST_RESULT;
}

