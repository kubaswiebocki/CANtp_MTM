#include "Test.h"

void Test_CanTp_Init(enum CanTp_StateType, enum expected)
{
	if(expected == CanTp_StateType)
	{
		printf("n\%s Passed");
	}
	else
	{
		printf("n\%s FAILED expected: %d CanTp_StateType: %d", expected, CanTp_StateType);
	}
	
}

void Test_CanTp_Shutdown(enum CanTp_StateType, enum expected)
{
	if(expected == CanTp_StateType)
	{
		printf("n\%s Passed");
	}
	else
	{
		printf("n\%s FAILED expected: %d CanTp_StateType: %d", expected, CanTp_StateType);
	}
	
}

void test()
{

Test_CanTp_Init(CANTP_ON, CanTp_Init(const CanTpConfig *CfgPtr));   
Test_CanTp_Shutdown(CANTP_OFF,CanTp_Shutdown());       
}

void all_tests()
{
  test();   
}


int main()
{
	test();
}