#include <stdlib.h>
#include <ndm/sys.h>
#include <ndm/time.h>
#include "test.h"

int main()
{
	struct timespec t = NDM_TIME_ZERO;
	struct timespec s;
	struct timespec u;
	struct timeval v;
	struct timespec min;
	struct timespec max;

	NDM_TEST_BREAK_IF(!ndm_time_init());

	NDM_TEST(ndm_time_is_zero(&t));

	ndm_time_get(&t);
	ndm_sys_sleep_msec(100);
	ndm_time_get(&s);

	NDM_TEST(!ndm_time_is_zero(&t));
	NDM_TEST(!ndm_time_is_zero(&s));
	NDM_TEST(!ndm_time_equal(&t, &s));
	NDM_TEST(ndm_time_less(&t, &s));
	NDM_TEST(ndm_time_less_or_equal(&t, &s));
	NDM_TEST(ndm_time_greater(&s, &t));
	NDM_TEST(ndm_time_greater_or_equal(&s, &t));

	ndm_time_get_monotonic(&t);
	ndm_sys_sleep_msec(100);
	ndm_time_get_monotonic(&s);

	NDM_TEST(!ndm_time_is_zero(&t));
	NDM_TEST(!ndm_time_is_zero(&s));
	NDM_TEST(!ndm_time_equal(&t, &s));
	NDM_TEST(ndm_time_less(&t, &s));
	NDM_TEST(ndm_time_less_or_equal(&t, &s));
	NDM_TEST(ndm_time_greater(&s, &t));
	NDM_TEST(ndm_time_greater_or_equal(&s, &t));

	u = t;

	ndm_time_add(&t, &s);
	ndm_time_add_msec(&t, 1000);
	ndm_time_sub(&t, &s);
	ndm_time_sub_msec(&t, 1000);

	NDM_TEST(ndm_time_equal(&u, &t));

	/* second checks */

	u.tv_sec = 100;
	u.tv_nsec = 500000;

	s.tv_sec = 80;
	s.tv_nsec = 8000000;

	ndm_time_sub(&u, &s);
	NDM_TEST(u.tv_sec == 19 && u.tv_nsec == 992500000);

	ndm_time_add_sec(&u, 2);
	NDM_TEST(u.tv_sec == 21 && u.tv_nsec == 992500000);

	ndm_time_sub_sec(&u, 3);
	NDM_TEST(u.tv_sec == 18 && u.tv_nsec == 992500000);

	NDM_TEST(ndm_time_to_sec(&u) == 19);

	u.tv_nsec = 500000000;
	NDM_TEST(ndm_time_to_sec(&u) == 19);

	u.tv_nsec = 499999999;
	NDM_TEST(ndm_time_to_sec(&u) == 18);

	ndm_time_from_sec(&u, 214);
	NDM_TEST(u.tv_sec == 214 && u.tv_nsec == 0);

	/* millisecond checks */

	u.tv_sec = 100;
	u.tv_nsec = 200200200;
	ndm_time_add_msec(&u, 9900);
	NDM_TEST(u.tv_sec == 110 && u.tv_nsec == 100200200);

	ndm_time_sub_msec(&u, 1100);
	NDM_TEST(u.tv_sec == 109 && u.tv_nsec == 200200);

	NDM_TEST(ndm_time_to_msec(&u) == 109000);

	u.tv_nsec = 500000;
	NDM_TEST(ndm_time_to_msec(&u) == 109001);

	u.tv_nsec = 499999;
	NDM_TEST(ndm_time_to_msec(&u) == 109000);

	/* microsecond checks */

	u.tv_sec = 100;
	u.tv_nsec = 200200200;
	ndm_time_add_usec(&u, 9900900);
	NDM_TEST(u.tv_sec == 110 && u.tv_nsec == 101100200);

	ndm_time_sub_usec(&u, 1100100);

	NDM_TEST(u.tv_sec == 109 && u.tv_nsec == 1000200);

	NDM_TEST(ndm_time_to_usec(&u) == 109001000);

	u.tv_nsec = 500;
	NDM_TEST(ndm_time_to_usec(&u) == 109000001);

	u.tv_nsec = 499;
	NDM_TEST(ndm_time_to_usec(&u) == 109000000);

	/* nanosecond checks */

	u.tv_sec = 10;
	u.tv_nsec = 200200200;
	ndm_time_add_nsec(&u, 2900900200);
	NDM_TEST(u.tv_sec == 13 && u.tv_nsec == 101100400);

	ndm_time_sub_nsec(&u, 1211800100);

	NDM_TEST(u.tv_sec == 11 && u.tv_nsec == 889300300);

	NDM_TEST(ndm_time_to_nsec(&u) == 11889300300ll);

	u.tv_nsec = 1;
	NDM_TEST(ndm_time_to_nsec(&u) == 11000000001ll);

	/* timeval checks */

	u.tv_sec = 10;
	u.tv_nsec = 200200200;

	v.tv_sec = 2;
	v.tv_usec = 900900;

	ndm_time_add_timeval(&u, &v);
	NDM_TEST(u.tv_sec == 13 && u.tv_nsec == 101100200);

	v.tv_sec = 1;
	v.tv_usec = 211800;

	ndm_time_sub_timeval(&u, &v);

	NDM_TEST(u.tv_sec == 11 && u.tv_nsec == 889300200);

	ndm_time_to_timeval(&v, &u);
	NDM_TEST(v.tv_sec == 11 && v.tv_usec == 889300);

	ndm_time_from_timeval(&u, &v);
	NDM_TEST(u.tv_sec == 11 && u.tv_nsec == 889300000);

	ndm_time_get_min(&min);
	ndm_time_get_max(&max);

	NDM_TEST(min.tv_sec - 1 == max.tv_sec);
	NDM_TEST(llabs(min.tv_nsec) < NDM_TIME_PREC);
	NDM_TEST(llabs(max.tv_nsec) < NDM_TIME_PREC);

	return NDM_TEST_RESULT;
}

