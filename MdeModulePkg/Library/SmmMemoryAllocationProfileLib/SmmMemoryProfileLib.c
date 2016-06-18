/** @file
  Support routines for memory profile for Smm phase drivers.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php.                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <PiSmm.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>

#include <Guid/MemoryProfile.h>

EDKII_MEMORY_PROFILE_PROTOCOL     *mLibProfileProtocol;
EDKII_SMM_MEMORY_PROFILE_PROTOCOL *mLibSmmProfileProtocol;

/**
  Check whether the start address of buffer is within any of the SMRAM ranges.

  @param[in]  Buffer   The pointer to the buffer to be checked.

  @retval     TURE     The buffer is in SMRAM ranges.
  @retval     FALSE    The buffer is out of SMRAM ranges.
**/
BOOLEAN
EFIAPI
BufferInSmram (
  IN VOID *Buffer
  );

/**
  The constructor function initializes memory profile for SMM phase.

  @param ImageHandle    The firmware allocated handle for the EFI image.
  @param SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
SmmMemoryProfileLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;

  //
  // Locate Profile Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEdkiiMemoryProfileGuid,
                  NULL,
                  (VOID **)&mLibProfileProtocol
                  );
  if (EFI_ERROR (Status)) {
    mLibProfileProtocol = NULL;
  }

  Status = gSmst->SmmLocateProtocol (
                    &gEdkiiSmmMemoryProfileGuid,
                    NULL,
                    (VOID **)&mLibSmmProfileProtocol
                    );
  if (EFI_ERROR (Status)) {
    mLibSmmProfileProtocol = NULL;
  }

  return EFI_SUCCESS;
}

/**
  Record memory profile of multilevel caller.

  @param[in] CallerAddress      Address of caller.
  @param[in] Action             Memory profile action.
  @param[in] MemoryType         Memory type.
                                EfiMaxMemoryType means the MemoryType is unknown.
  @param[in] Buffer             Buffer address.
  @param[in] Size               Buffer size.
  @param[in] ActionString       String for memory profile action.
                                Only needed for user defined allocate action.

  @return EFI_SUCCESS           Memory profile is updated.
  @return EFI_UNSUPPORTED       Memory profile is unsupported,
                                or memory profile for the image is not required,
                                or memory profile for the memory type is not required.
  @return EFI_ACCESS_DENIED     It is during memory profile data getting.
  @return EFI_ABORTED           Memory profile recording is not enabled.
  @return EFI_OUT_OF_RESOURCES  No enough resource to update memory profile for allocate action.
  @return EFI_NOT_FOUND         No matched allocate info found for free action.

**/
EFI_STATUS
EFIAPI
MemoryProfileLibRecord (
  IN PHYSICAL_ADDRESS           CallerAddress,
  IN MEMORY_PROFILE_ACTION      Action,
  IN EFI_MEMORY_TYPE            MemoryType,
  IN VOID                       *Buffer,
  IN UINTN                      Size,
  IN CHAR8                      *ActionString OPTIONAL
  )
{
  if (BufferInSmram (Buffer)) {
    if (mLibSmmProfileProtocol == NULL) {
      return EFI_UNSUPPORTED;
    }
    return mLibSmmProfileProtocol->Record (
                                     mLibSmmProfileProtocol,
                                     CallerAddress,
                                     Action,
                                     MemoryType,
                                     Buffer,
                                     Size,
                                     ActionString
                                     );
  } else {
    if (mLibProfileProtocol == NULL) {
      return EFI_UNSUPPORTED;
    }
    return mLibProfileProtocol->Record (
                                  mLibProfileProtocol,
                                  CallerAddress,
                                  Action,
                                  MemoryType,
                                  Buffer,
                                  Size,
                                  ActionString
                                  );
  }
}

