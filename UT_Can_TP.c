/** ==================================================================================================================*\
  @file UT_Can_TP.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"   

/**
  @brief Test inicjalizacji

  Funkcja testująca inicjalizacji CanTp.
*/

void Test_Of_CanTp_Init(void)
{
	
}

/**
  @brief Test wyłączenia

  Funkcja testująca wyłączenie CanTp.
*/
void Test_Of_CanTp_Shutdown(void)
{
	CanTp_StateType CanTpState = CANTP_ON;

	CanTp_Shutdown();

    TEST_CHECK(CanTpState == CANTP_ON);
}

/**
  @brief Test wersji

  Funkcja testująca zapisywana wersję CanTp.
*/
void Test_Of_CanTp_GetVersionInfo(void)
{
	Std_VersionInfoType versioninfo;
	CanTp_GetVersionInfo(&versioninfo);
	TEST_CHECK(versioninfo.sw_major_version == 0x00u);
	TEST_CHECK(versioninfo.sw_minor_version == 0x01u);
	TEST_CHECK(versioninfo.sw_patch_version == 0x00u);
	TEST_CHECK(versioninfo.vendorID == 0x00u);
    TEST_CHECK(versioninfo.moduleID == 0x0Eu);

	TEST_CHECK(versioninfo.sw_major_version != 0x01u);
	TEST_CHECK(versioninfo.sw_minor_version != 0x00u);
	TEST_CHECK(versioninfo.sw_patch_version != 0x01u);
	TEST_CHECK(versioninfo.vendorID != 0x01u);
    TEST_CHECK(versioninfo.moduleID != 0x00u);

}

TEST_LIST = {
    // { "Test of CanTp_Init", Test_Of_CanTp_Init },
    { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	// { "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};