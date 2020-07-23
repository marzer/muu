#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import sys
import os.path as path
import utils
import math
import random
import decimal



def int_literal(val, bits, always_hex = False):
	if not utils.is_pow2(bits):
		bits = utils.next_power_of_2(bits)
	if bits > 64:
		vals = []
		for i in range(0, int(bits / 64)):
			vals.insert(0, int_literal(val & 0xFFFFFFFFFFFFFFFF, 64, always_hex))
			val = val >> 64
		return 'pack({})'.format(', '.join(vals))
	else:
		mask = int((1 << bits) - 1)
		if bits > 32 or always_hex:
			return '0x{:0{}X}_u{}'.format(val & mask, int(bits / 4), bits)
		else:
			return '0b{:0{}b}_u{}'.format(val & mask, bits, bits)



class FloatTraits(object):

	def __init__(self, padding_bits, sign_bits, exponent_bits, integer_part_bits, significand_bits):
		assert padding_bits >= 0
		#assert 0 <= sign_bits <= 1
		assert sign_bits == 1
		assert exponent_bits > 0
		assert 0 <= integer_part_bits <= 1
		assert significand_bits > 0

		self.total_bits = padding_bits + sign_bits + exponent_bits + integer_part_bits + significand_bits
		self.padding_bits = padding_bits
		self.sign_bits = sign_bits
		self.exponent_bits = exponent_bits
		self.integer_part_bits = integer_part_bits
		self.significand_bits = significand_bits
		self.digits = max(integer_part_bits,1) + significand_bits
		self.digits10 = int(math.floor((self.digits - 1) * math.log10(2)))
		self.int_blittable = utils.is_pow2(self.total_bits)

		bit_fill = lambda b: (1 << b) - 1
		self.sign_mask = (1 << (self.total_bits-padding_bits-1)) if sign_bits == 1 else 0
		self.exponent_mask = bit_fill(exponent_bits) << (integer_part_bits + significand_bits)
		self.integer_part_mask = 1 << significand_bits if integer_part_bits == 1 else 0
		self.significand_mask = bit_fill(significand_bits)
		self.inf_nan_mask = self.exponent_mask | self.integer_part_mask
		self.quiet_nan_mask = self.sign_mask | self.inf_nan_mask | 1 | (1 << (self.significand_bits - 1))
		self.signalling_nan_mask = self.sign_mask | self.inf_nan_mask | 1

	def constant(self, val, allow_blitting = True):
		if self.int_blittable and allow_blitting:
			return int_literal(val, self.total_bits)
		else:
			elem_bits = self.total_bits
			if not utils.is_pow2(elem_bits):
				elem_bits = utils.next_power_of_2(elem_bits) >> 1
			while (self.total_bits % elem_bits) != 0 or elem_bits > 64:
				elem_bits = elem_bits >> 1
			elems = []
			for i in range(0, int(self.total_bits / elem_bits)):
				elems.append(int_literal(val, elem_bits, always_hex = True))
				val = val >> elem_bits
			return 'array{{ {} }}'.format(', '.join(elems))




def write_float_data(file, traits):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	D = decimal.Decimal
	rounded = lambda d,p=traits.digits10: d.quantize(D(10) ** -p)

	write('\ttemplate <>')
	write('\tstruct float_test_data_by_traits<{}, {}>'.format(traits.total_bits, traits.digits))
	write('\t{')

	write('\t\tstatic constexpr long double values[] = ')
	write('\t\t{',end='')
	per_line = max(int(108 / (traits.digits10 + 5)), 1)
	decimal.getcontext().prec = 256
	sum = D(0)
	random.seed(4815162342)
	denom = 10 ** traits.digits10
	denom = (denom, D(denom * 2))
	for i in range(0, 400):
		val = rounded(D(random.randrange(1, denom[0])) / denom[1])
		sum = sum + val
		if i > 0:
			write(', ',end='')
		if i % per_line == 0:
			write('\n\t\t\t',end='')
		write('{}L'.format(val), end='')
	write('\n\t\t};')
	write('')

	delta = (D(10) ** -(max(int(round(3.0*traits.digits10/4.0))+1,2)))
	# write('\t\t// {}'.format(D(1) - delta))
	# write('\t\t// {}'.format(D(1) + delta))
	write('\t\tstatic constexpr long double values_sum_low  = {}L;'.format(rounded(sum * (D(1) - delta))))
	write('\t\tstatic constexpr long double values_sum      = {}L;'.format(rounded(sum)))
	write('\t\tstatic constexpr long double values_sum_high = {}L;'.format(rounded(sum * (D(1) + delta))))
	write('')

	if traits.total_bits > 64 and traits.int_blittable:
		write('\t\t#if MUU_HAS_INT{}'.format(traits.total_bits))

	blit = traits.int_blittable
	sca = 'static constexpr auto'
	for i in range(0,2):
		write('\t\t{} int_blittable    = {};'.format(sca, 'true' if blit else 'false'))
		write('\t\t{} bits_sign        = {};'.format(sca, traits.constant(traits.sign_mask, blit)))
		write('\t\t{} bits_exponent    = {};'.format(sca, traits.constant(traits.exponent_mask, blit)))
		if traits.integer_part_mask != 0:
			write('\t\t{} bits_integer     = {};'.format(sca, traits.constant(traits.integer_part_mask, blit)))
		write('\t\t{} bits_mantissa    = {};'.format(sca, traits.constant(traits.significand_mask, blit)))
		write('\t\t{} bits_pos_inf     = {};'.format(sca, traits.constant(traits.inf_nan_mask, blit)))
		write('\t\t{} bits_neg_inf     = {};'.format(sca, traits.constant(traits.inf_nan_mask | traits.sign_mask, blit)))
		write('\t\t{} bits_pos_nan_min = {};'.format(sca, traits.constant(traits.inf_nan_mask | 1, blit)))
		write('\t\t{} bits_pos_nan_max = {};'.format(sca, traits.constant(traits.inf_nan_mask | traits.significand_mask, blit)))
		write('\t\t{} bits_neg_nan_min = {};'.format(sca, traits.constant(traits.inf_nan_mask | 1 | traits.sign_mask, blit)))
		write('\t\t{} bits_neg_nan_max = {};'.format(sca, traits.constant(traits.inf_nan_mask | traits.significand_mask | traits.sign_mask, blit)))
		write('\t\t{} bits_snan        = {};'.format(sca, traits.constant(traits.signalling_nan_mask, blit)))
		write('\t\t{} bits_qnan        = {};'.format(sca, traits.constant(traits.quiet_nan_mask, blit)))

		if traits.total_bits <= 64 or not blit:
			break;
		write('\t\t#else')
		blit = not blit

	if traits.total_bits > 64 and traits.int_blittable:
		write('\t\t#endif // MUU_HAS_INT{}'.format(traits.total_bits))

	write('\t};')
	write('')


def main():
	
	file_path = path.join(utils.get_script_folder(), '..', 'tests', 'float_test_data.h')
	print("Writing to {}".format(file_path))
	with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
		indent = 0
		write = lambda txt: print(('\t' * indent) + txt if txt != '' else '', file=file)

		# preamble
		write('// This file is a part of muu and is subject to the the terms of the MIT license.')
		write('// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>')
		write('// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		write('// SPDX-License-Identifier: MIT')
		write('//-----')
		write('// this file was generated by generate_float_tests_header.py - do not modify it directly')
		write('')
		write('#pragma once')
		write('#include "settings.h"')
		write('#include "../include/muu/core.h"')
		write('')
		write('#ifndef MUU_HAS_INT256')
		write('\t#define MUU_HAS_INT256 0')
		write('#endif')
		write('')
		write('MUU_NAMESPACE_START')
		write('{')
		indent = indent + 1 

		# data tables
		write('template <size_t TotalBits, size_t SignificandBits>')
		write('struct float_test_data_by_traits;')
		write('')

		# (padding_bits, sign_bits, exponent_bits, integer_part_bits, significand_bits)

		write_float_data(file, FloatTraits( 0, 1,  5, 0,  10)) # 16-bit 'half'
		write_float_data(file, FloatTraits( 0, 1,  8, 0,  23)) # 32-bit float
		write_float_data(file, FloatTraits( 0, 1, 11, 0,  52)) # 64-bit double
		write_float_data(file, FloatTraits( 0, 1, 15, 1,  63)) # 80-bit long double
		write_float_data(file, FloatTraits(48, 1, 15, 1,  63)) # 80-bit long double stored as 128 bits

		#write_float_data(file, FloatTraits(128,  0, 15, 0, 113))
		#write_float_data(file, FloatTraits(256,  0, 19, 0, 237))

		# alias by type
		write('template <typename T>')
		write('struct float_test_data : float_test_data_by_traits<sizeof(T) * CHAR_BIT, std::numeric_limits<T>::digits> {};')

		indent = indent - 1 
		write('}')
		write('MUU_NAMESPACE_END')
		

if __name__ == '__main__':
	utils.run(main)
