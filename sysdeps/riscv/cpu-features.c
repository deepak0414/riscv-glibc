/* Initialize CPU feature data.
   This file is part of the GNU C Library.
   Copyright (C) 2008-2024 Free Software Foundation, Inc.

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

#ifndef _CPU_FEATURES_RISCV_H
#define _CPU_FEATURES_RISCV_H

# define TUNABLE_NAMESPACE cpu
# include <cpu-tunables.c>

# ifdef __riscv_zicfilp
extern void TUNABLE_CALLBACK (set_riscv_cfi_lp) (tunable_val_t *)
  attribute_hidden;
# endif
# ifdef __riscv_zicfiss
extern void TUNABLE_CALLBACK (set_riscv_cfi_ss) (tunable_val_t *)
  attribute_hidden;
# endif

static inline void
init_cpu_features (void)
{
# ifdef __riscv_zicfilp
  TUNABLE_GET (riscv_cfi_lp, tunable_val_t *,
	       TUNABLE_CALLBACK (set_riscv_cfi_lp));
# endif
# ifdef __riscv_zicfiss
  TUNABLE_GET (riscv_cfi_ss, tunable_val_t *,
	       TUNABLE_CALLBACK (set_riscv_cfi_ss));
# endif
}
#endif /* _CPU_FEATURES_RISCV_H  */
