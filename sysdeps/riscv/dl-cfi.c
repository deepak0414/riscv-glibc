/* RISC-V CFI extensions (zicfilp/zicfiss) functions.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include "feature-control.h"
#include <asm-generic/errno-base.h>
#include <unistd.h>
#include <libintl.h>
#include <ldsodefs.h>
#include <dl-cfi.h>
#include <sys/mman.h>

static void
dl_check_legacy_object (struct link_map *m, unsigned int *feature_1)
{
  /* Iterate through the dependencies and disable if needed here  */
  struct link_map *l = NULL;
  unsigned int i;
  i = m->l_searchlist.r_nlist;
  while (i-- > 0)
    {
      /* Check each shared object to see if shadow stack and landing pad
         are enabled.  */
      l = m->l_initfini[i];

      if (l->l_init_called)
        continue;

#ifdef SHARED
      /* Skip check for ld.so since it has the features enabled. The
         features will be disabled later if they are not enabled in
	 executable.  */
      if (l == &GL(dl_rtld_map)
          || l->l_real == &GL(dl_rtld_map))
        continue;
#endif /* SHARED */

      *feature_1 &= l->l_riscv_feature_1_and;
    }
#ifdef __riscv_zicfilp
  if (GL(dl_riscv_feature_control).lp == cfi_always_on)
    *feature_1 |= GNU_PROPERTY_RISCV_FEATURE_1_FCFI;
#endif
#ifdef __riscv_zicfiss
  if (GL(dl_riscv_feature_control).ss == cfi_always_on)
    *feature_1 |= GNU_PROPERTY_RISCV_FEATURE_1_BCFI;
#endif
}

#ifdef SHARED
static void
dl_cfi_check_startup (struct link_map *m, unsigned int *feature_1)
{
#ifdef __riscv_zicfilp
  if (GL(dl_riscv_feature_control).lp == cfi_always_off)
    *feature_1 &= ~GNU_PROPERTY_RISCV_FEATURE_1_FCFI;
#endif
#ifdef __riscv_zicfiss
  if (GL(dl_riscv_feature_control).ss == cfi_always_off)
    *feature_1 &= ~GNU_PROPERTY_RISCV_FEATURE_1_BCFI;
#endif

  dl_check_legacy_object (m, feature_1);

  /* Update GL(dl_riscv_feature_1)  */
  GL(dl_riscv_feature_1) = *feature_1;
}
#endif /* SHARED */

static void
dl_cfi_check_dlopen (struct link_map *m)
{
}

attribute_hidden void
_dl_cfi_setup_features (unsigned int feature_1)
{
  /* Since prctl could fail to enable some features
     use prctl to get enabled features again and sync it back  */
#ifdef __riscv_zicfilp
  if (feature_1 & GNU_PROPERTY_RISCV_FEATURE_1_FCFI)
    INTERNAL_SYSCALL_CALL (prctl, PR_SET_INDIR_BR_LP_STATUS,
                           PR_INDIR_BR_LP_ENABLE);
#endif /* __riscv_zicfilp  */
  /* FIXME: Read enabled features from kernel and re-sync  */
}

/* Enable CFI for l and its dependencies.  */
void
_dl_cfi_check (struct link_map *l, const char *program)
{
    /* As this point we have parsed the gnu properties
       For dynamic binary we should verify the dependencies here
       FIXME: Implement different policy for supporting legacy binaries  */
  unsigned int feature_1;
#if defined SHARED && defined RTLD_START_ENABLE_RISCV_CFI
  if (program)
    {
      GL(dl_riscv_feature_1) = l->l_riscv_feature_1_and;
    }
#endif /* SHARED */

  unsigned int supported_exts = 0;
  unsigned int always_on_exts = 0;

#ifdef __riscv_zicfilp
  supported_exts += 1;
  always_on_exts += (GL(dl_riscv_feature_control).lp == cfi_always_on);
#endif
#ifdef __riscv_zicfiss
  supported_exts += 1;
  always_on_exts += (GL(dl_riscv_feature_control).ss == cfi_always_on);
#endif

  feature_1 = GL(dl_riscv_feature_1);

  /* No legacy check needed if all cfi exts are always on */
  if (supported_exts == always_on_exts)
    return;

  /* No legacy check needed if all cfi exts are off */
  if (feature_1 == 0)
    return;

#ifdef SHARED
  if (program)
    dl_cfi_check_startup (l, &feature_1);
  else
#endif /* SHARED */
    dl_cfi_check_dlopen (l);
}
