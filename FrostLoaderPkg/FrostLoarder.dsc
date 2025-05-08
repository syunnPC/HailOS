[Defines]
  PLATFORM_NAME           = FrostLoader
  PLATFORM_GUID           = 9add1f9d-8a0c-4adb-b3dc-37414b57c0a9
  PLATFORM_VERSION        = 0.1
  DSC_SPECIFICATION       = 0x00010005
  OUTPUT_DIRECTORY        = Build/FrostLoader
  SUPPORTED_ARCHITECTURES = X64
  BUILD_TARGETS           = DEBUG|RELEASE
  SKUID_IDENTIFIER        = DEFAULT

[LibraryClasses]
  UefiLib
  UefiBootServicesTableLib
  MemoryAllocationLib
  BaseMemoryLib
  PrintLib

[LibraryClasses.common]
  NULL|MdePkg/Library/UefiLib/UefiLib.inf
  NULL|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  NULL|MdePkg/Library/MemoryAllocationLib/MemoryAllocationLib.inf
  NULL|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  NULL|MdePkg/Library/PrintLib/PrintLib.inf

[Components]
  FrostLoaderPkg/FrostLoader.inf