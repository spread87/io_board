#include "sci.h"

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

//static int total_nodes;

int ksum_local_buf[8];
volatile int valid_num = 0;

int subset_sum(int s[], int t[],
		int s_size, int t_size,
		int sum, int ite,
		int const target_sum)
{
	if (target_sum == sum) {
		valid_num = t_size;
		return 1;
	}
	else {
		for (int i = ite; i < s_size; i++) {
			t[t_size] = i;
			if (!s[i])
				continue;
			if (subset_sum(s, t, s_size, t_size + 1, sum + s[i], i + 1, target_sum)) {
				return 1;
			}
		}
		return 0;
	}
}


int generateSubsets(int s[], int size, int target_sum)
{
	return subset_sum(s, ksum_local_buf, size, 0, 0, 0, target_sum);
}

unsigned short get_calc_result(void)
{
	int i;
	volatile unsigned short local_channel = 0;

	for (i = 0; i < valid_num; i++)
	{
		local_channel |= 1 << ksum_local_buf[i];
	}
	return local_channel;
}

int ksum_test()
{
	int weights[] = {20, 18, 15, 12, 10, 9, 7, 5};
	int size = ARRAYSIZE(weights);

	generateSubsets(weights, size, 19);
	return 0;
}
