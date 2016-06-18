/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPropertyPersistence.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkStringProperty.h"
#include "mitkIOMimeTypes.h"
#include <mitkNumericConstants.h>
#include <mitkEqual.h>

#include <algorithm>
#include <limits>

class mitkPropertyPersistenceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyPersistenceTestSuite);

  MITK_TEST(AddInfo);
  MITK_TEST(GetInfos);
  MITK_TEST(GetInfos_mime);
  MITK_TEST(GetInfosByKey);
  MITK_TEST(HasInfos);
  MITK_TEST(RemoveAllInfos);
  MITK_TEST(RemoveInfos);
  MITK_TEST(RemoveInfos_withMime);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PropertyPersistenceInfo::Pointer info1;
  mitk::PropertyPersistenceInfo::Pointer info2;
  mitk::PropertyPersistenceInfo::Pointer info3;
  mitk::PropertyPersistenceInfo::Pointer info4;
  mitk::PropertyPersistenceInfo::Pointer info5;
  mitk::PropertyPersistenceInfo::Pointer info6;

  mitk::PropertyPersistenceInfo::Pointer infoX;
  mitk::PropertyPersistenceInfo::Pointer infoX2;

  std::string prop1;
  std::string prop2;
  std::string prop3;
  std::string prop4;
  std::string prop5;
  std::string prop6;

  std::string propX;
  std::string keyX;
  std::string propXTemplate;
  std::string keyXTemplate;
  std::string propX2;

  mitk::IPropertyPersistence* service;

  static bool checkExistance(const mitk::PropertyPersistence::InfoResultType& infos, const mitk::PropertyPersistenceInfo* info)
  {
    auto predicate = [info](const mitk::PropertyPersistenceInfo::ConstPointer& x){return infosAreEqual(info, x); };

    auto finding = std::find_if(infos.begin(), infos.end(), predicate);

    bool result = finding != infos.end();
    return result;
  }

  static bool infosAreEqual(const mitk::PropertyPersistenceInfo* ref, const mitk::PropertyPersistenceInfo* info)
  {
    bool result = true;

    if (!info || !ref)
    {
      return false;
    }

    result = result && ref->GetName() == info->GetName();
    result = result && ref->GetKey() == info->GetKey();
    result = result && ref->GetMimeTypeName() == info->GetMimeTypeName();
    return result;
  }

public:

  void setUp() override
  {
    service = mitk::CreateTestInstancePropertyPersistence();

    prop1 = "prop1";
    prop2 = "prop1";
    prop3 = "prop1";
    prop4 = "prop4";
    prop5 = "prop5";

    propX = "prop(\\d*)";
    keyX = "key(\\d*)";
    propXTemplate = "prop$1";
    keyXTemplate = "key.$1";

    propX2 = "otherprop(\\d*)";

    info1 = mitk::PropertyPersistenceInfo::New();
    info1->SetNameAndKey(prop1, "key1");
    info2 = mitk::PropertyPersistenceInfo::New(prop2, "mime2");
    info2->SetNameAndKey(prop2, "key2");
    info3 = mitk::PropertyPersistenceInfo::New(prop3, "mime3");
    info3->SetNameAndKey(prop3, "key3");
    info4 = mitk::PropertyPersistenceInfo::New(prop4, "mime2");
    info4->SetNameAndKey(prop4, "key2");
    info5 = mitk::PropertyPersistenceInfo::New(prop5, "mime5");
    info5->SetNameAndKey(prop5, "key5");

    infoX = mitk::PropertyPersistenceInfo::New("","mimeX");
    infoX->UseRegEx(propX, propXTemplate, keyX, keyXTemplate);

    infoX2 = mitk::PropertyPersistenceInfo::New();
    infoX2->UseRegEx(propX2, propXTemplate);

    service->AddInfo(info1, false);
    service->AddInfo(info2, false);
    service->AddInfo(info3, false);
    service->AddInfo(info4, false);
    service->AddInfo(info5, false);
    service->AddInfo(infoX, false);
    service->AddInfo(infoX2, false);
  }

  void tearDown() override
  {
    delete service;
  }

  void AddInfo()
  {
    mitk::PropertyPersistenceInfo::Pointer info2_new = mitk::PropertyPersistenceInfo::New(prop2, "otherMime");
    info2_new->SetNameAndKey(prop2, "newKey");
    mitk::PropertyPersistenceInfo::Pointer info2_otherKey = mitk::PropertyPersistenceInfo::New("prop2", "mime2");
    info2_otherKey->SetNameAndKey(prop2, "otherKey");
    mitk::PropertyPersistenceInfo::Pointer info_newPropNKey = mitk::PropertyPersistenceInfo::New("", "otherMime");
    info_newPropNKey->SetNameAndKey("newProp", "newKey");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> no adding", !service->AddInfo(info2_otherKey, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> no adding -> key should not be changed.", service->GetInfos(prop2, "mime2", false).front()->GetKey() == "key2");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (overwrite) -> adding", service->AddInfo(info2_otherKey, true));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of already existing info (no overwrite) -> adding -> key should be changed.", service->GetInfos(prop2, "mime2", false).front()->GetKey() == "otherKey");

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (other mime type; no overwrite) -> adding", service->AddInfo(info2_new, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (other mime type; no overwrite) -> adding -> info exists.", !service->GetInfos(prop2, "otherMime", false).empty());

    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (new prop name; no overwrite) -> adding", service->AddInfo(info_newPropNKey, false));
    CPPUNIT_ASSERT_MESSAGE("Testing addinfo of info (new prop name; no overwrite) -> adding ->info exists.", !service->GetInfos("newProp", "otherMime", false).empty());
  }

  void GetInfos()
  {
    mitk::PropertyPersistence::InfoResultType infos = service->GetInfos(prop1, false);
    CPPUNIT_ASSERT(infos.size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info1));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info2));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info3));

    infos = service->GetInfos(prop4, false);
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info4));

    infos = service->GetInfos("unkown", false);
    CPPUNIT_ASSERT_MESSAGE("Check size of result for unkown prop.", infos.empty());

    infos = service->GetInfos("prop101", false);
    CPPUNIT_ASSERT(infos.empty());

    infos = service->GetInfos("prop101", true);
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check Name of expected element 1.", infos.front()->GetName() == "prop101");
    CPPUNIT_ASSERT_MESSAGE("Check Key of expected element 1.", infos.front()->GetKey() == "key.101");
    CPPUNIT_ASSERT_MESSAGE("Check MimeTypeName of expected element 1.", infos.front()->GetMimeTypeName() == "mimeX");
  }

  void GetInfosByKey()
  {
    mitk::PropertyPersistence::InfoResultType infos = service->GetInfosByKey("key2", false);
    CPPUNIT_ASSERT(infos.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info2));
    CPPUNIT_ASSERT_MESSAGE("Check expected element 2.", checkExistance(infos, info4));

    infos = service->GetInfosByKey("key5", false);
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check expected element 1.", checkExistance(infos, info5));

    infos = service->GetInfosByKey("unkownkey", false);
    CPPUNIT_ASSERT_MESSAGE("Check size of result for unkown key.", infos.empty());

    infos = service->GetInfosByKey("key101", false);
    CPPUNIT_ASSERT_MESSAGE("Check size of result for key101.", infos.empty());

    infos = service->GetInfosByKey("key101", true);
    CPPUNIT_ASSERT(infos.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check Name of expected element 1.", infos.front()->GetName() == "prop101");
    CPPUNIT_ASSERT_MESSAGE("Check Key of expected element 1.", infos.front()->GetKey() == "key101");
    CPPUNIT_ASSERT_MESSAGE("Check MimeTypeName of expected element 1.", infos.front()->GetMimeTypeName() == "mimeX");
  }

  void GetInfos_mime()
  {
    mitk::PropertyPersistence::InfoResultType infos = service->GetInfos(prop1, "mime2", false, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (existing element, no wildcard allowed, wildcard exists).", infosAreEqual(info2, infos.front()));
    infos = service->GetInfos(prop1, "mime2", true, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (existing element, wildcard allowed, wildcard exists).", infosAreEqual(info2, infos.front()));
    infos = service->GetInfos(prop1, "unknownmime", false, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting element, no wildcard allowed, wildcard exists).", infos.empty());
    infos = service->GetInfos(prop1, "unknownmime", true, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting element, wildcard allowed, wildcard exists).", infosAreEqual(info1, infos.front()));

    infos = service->GetInfos(prop4, "unknownmime", false, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting element, no wildcard allowed).", infos.empty());
    infos = service->GetInfos(prop4, "unknownmime", true, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting element, wildcard allowed).", infos.empty());

    infos = service->GetInfos("prop101", "unknownmime", false, true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting mime, no wildcard allowed, regex allowed).", infos.empty());

    infos = service->GetInfos("prop101", "mimeX", false, true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (existing mime, no wildcard allowed, regex allowed).", infos.size() == 1);

    infos = service->GetInfos("otherprop", "unknownmime", false, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting mime, no wildcard allowed, no regex allowed).", infos.empty());

    infos = service->GetInfos("otherprop", "unknownmime", true, false);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting mime, wildcard allowed, no regex allowed).", infos.empty());

    infos = service->GetInfos("otherprop", "unknownmime", false, true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting mime, no wildcard allowed, regex allowed).", infos.empty());

    infos = service->GetInfos("otherprop", "unknownmime", true, true);
    CPPUNIT_ASSERT_MESSAGE("Check GetInfos (inexisting mime, wildcard allowed, regex allowed).", infos.size() == 1);
  }

  void HasInfos()
  {
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (unkown prop)", !service->HasInfos("unkownProp"));
  }

  void RemoveAllInfos()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveAllInfos());
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", !service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", !service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", !service->HasInfos(prop5));
  }


  void RemoveInfos()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop1)", !service->HasInfos(prop1));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop4)", !service->HasInfos(prop4));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5));
    CPPUNIT_ASSERT_MESSAGE("Check HasInfos (prop5)", !service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos("unknown_prop"));
  }

  void RemoveInfos_withMime()
  {
    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop1, "mime2"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos if info was removed",service->GetInfos(prop1, "mime2", false).empty());
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if other info of same property name still exists", !service->GetInfos(prop1, "mime3", false).empty());
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if other info of other property name but same mime still exists", !service->GetInfos(prop4, "mime2", false).empty());

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5, "wrongMime"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos on prop 5 with wrong mime", service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos(prop5, "mime5"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos on prop 5", !service->HasInfos(prop5));

    CPPUNIT_ASSERT_NO_THROW(service->RemoveInfos("unkown_prop", "mime2"));
    CPPUNIT_ASSERT_MESSAGE("Check RemoveInfos, if unkown property name but exting mime was used", service->HasInfos(prop4));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyPersistence)
