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



__pi_and_tau = dict()
def pi_and_tau():
	global __pi_and_tau
	result = __pi_and_tau.get(decimal.getcontext().prec)
	if result is None:
		decimal.getcontext().prec += 4
		three = decimal.Decimal(3)
		lasts, t, s, n, na, d, da = 0, three, 3, 1, 0, 0, 24
		while s != lasts:
			lasts = s
			n, na = n+na, na+8
			d, da = d+da, da+32
			t = (t * n) / d
			s += t
		tau = s * decimal.Decimal(2)
		decimal.getcontext().prec -= 4
		result = (decimal.Decimal(s), decimal.Decimal(tau))
		__pi_and_tau[decimal.getcontext().prec] = result
	return result


def pi():
	return pi_and_tau()[0]


def tau():
	return pi_and_tau()[1]


__e = dict()
def e(power):
	global __e
	h = utils.multi_hash(decimal.getcontext().prec, power)
	result = __e.get(h)
	if result is None:
		decimal.getcontext().prec += 4
		i, lasts, s, fact, num = 0, 0, 1, 1, 1
		while s != lasts:
			lasts = s
			i += 1
			fact *= i
			num *= power
			s += num / fact
		decimal.getcontext().prec -= 4
		result = decimal.Decimal(s)
		__e[h] = result
	return result



def int_literal(val, bits, always_hex = False):
	if not utils.is_pow2(bits):
		bits = utils.next_power_of_2(bits)
	if bits > 64:
		vals = []
		for _ in range(0, int(bits / 64)):
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
		self.max_digits10 = int(math.ceil(1 + self.digits * math.log10(2)))
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
			for _ in range(0, int(self.total_bits / elem_bits)):
				elems.append(int_literal(val, elem_bits, always_hex = True))
				val = val >> elem_bits
			return 'std::array{{ {} }}'.format(', '.join(elems))

	def bit_representation(self,dec):
		return ""
		#D = decimal.Decimal
		#dec = dec.normalize()
		#
		#sign_bit = '1' if dec < D(0) else '0'
		#if dec < D(0):
		#	dec = -dec
		#assert dec >= D(0)
		#
		#integral = int(dec.to_integral_value(rounding=decimal.ROUND_FLOOR))
		#fractional = dec
		#if integral > 0:
		#	fractional = fractional - D(integral)
		#	if fractional < D(0):
		#		fractional = -fractional
		#assert integral >= 0
		#assert fractional >= D(0)
		#assert fractional < D(1)
		#assert (integral + fractional) == dec, f"{integral}.{fractional} == {dec}"
		#
		#integral_bits = ''
		#if integral == 0:
		#	integral_bits = '0'
		#else:
		#	bit = 1 << (self.significand_bits-1)
		#	while bit:
		#		if bit & integral:
		#			integral_bits = integral_bits + '1'
		#		elif len(integral_bits) > 0:
		#			integral_bits = integral_bits + '0'
		#		bit = bit >> 1
		#	pass
		#
		#fractional_bits = ''
		#prevPrec = decimal.getcontext().prec
		#p = self.max_digits10 + 5
		#if fractional.is_zero():
		#	fractional_bits = '0'
		#else:
		#	while p > 0:
		#		decimal.getcontext().prec = p
		#		fractional = (+fractional).normalize()
		#		f = (fractional * D(2)).normalize()
		#		if f >= D(1):
		#			fractional_bits = fractional_bits + '1'
		#			if f == D(1):
		#				break
		#			fractional = f - D(1)
		#		else:
		#			fractional_bits = fractional_bits + '0'
		#			fractional = f
		#		p = p - 1
		#		print(fractional)
		#decimal.getcontext().prec = prevPrec
		#
		#exponent_bits = ''
		#if integral == 0:
		#	exponent_bits = '0' * self.exponent_bits
		#else:
		#	exponent = len(exponent_bits)
		#	bit = 1 << (self.exponent_bits-1)
		#	while bit:
		#		if bit & integral:
		#			exponent_bits = exponent_bits + '1'
		#		else:
		#			exponent_bits = exponent_bits + '0'
		#		bit = bit >> 1
		#
		#mantissa_bits = (integral_bits + fractional_bits)[:self.significand_bits]
		#if len(mantissa_bits) < self.significand_bits:
		#	mantissa_bits = mantissa_bits + '0' * (self.significand_bits - len(mantissa_bits))
		#
		#return sign_bit + "'" + exponent_bits + "'" + mantissa_bits




def write_float_data(file, traits):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	D = decimal.Decimal
	rounded = lambda d,p=traits.digits10: d.quantize(D(10) ** -p)

	write('\ttemplate <>')
	write('\tstruct float_test_data_by_traits<{}, {}>'.format(traits.total_bits, traits.digits))
	write('\t{')

	type = 'long double'
	suffix = 'L'
	if traits.total_bits == 128 and traits.significand_bits == 112:
		type = 'float128_t'
		suffix = 'q'

	write(f'\t\tstatic constexpr {type} values[] = ')
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
		write('{}{}'.format(val, suffix), end='')
	write('\n\t\t};')
	write('')

	delta = (D(10) ** -(max(int(round(3.0*traits.digits10/4.0))+1,2)))
	# write('\t\t// {}'.format(D(1) - delta))
	# write('\t\t// {}'.format(D(1) + delta))
	write(f'\t\tstatic constexpr {type} values_sum_low      = {rounded(sum * (D(1) - delta))}{suffix};')
	write(f'\t\tstatic constexpr {type} values_sum          = {rounded(sum)}{suffix};')
	write(f'\t\tstatic constexpr {type} values_sum_high     = {rounded(sum * (D(1) + delta))}{suffix};')
	write('')

	print_constant = lambda name, value: \
		write(f'\t\tstatic constexpr {type} {name}{" " * (22 - len(name))}= {rounded(value, traits.max_digits10)}{suffix}; // {traits.bit_representation(value)}')
	print_constant('one_over_two',			D(1)/D(2))
	print_constant('one_over_three',		D(1)/D(3))
	print_constant('one_over_four',			D(1)/D(4))
	print_constant('one_over_five',			D(1)/D(5))
	print_constant('one_over_six',			D(1)/D(6))
	print_constant('root_one_over_two',		(D(1)/D(2)).sqrt())
	print_constant('root_one_over_three',	(D(1)/D(3)).sqrt())
	print_constant('root_one_over_four',	(D(1)/D(4)).sqrt())
	print_constant('root_one_over_five',	(D(1)/D(5)).sqrt())
	print_constant('root_one_over_six',		(D(1)/D(6)).sqrt())
	print_constant('two_over_three',		D(2)/D(3))
	print_constant('two_over_five',			D(2)/D(5))
	print_constant('root_two_over_three',	(D(2)/D(3)).sqrt())
	print_constant('root_two_over_five',	(D(2)/D(5)).sqrt())
	print_constant('three_over_two',		D(3)/D(2))
	print_constant('three_over_four',		D(3)/D(4))
	print_constant('three_over_five',		D(3)/D(5))
	print_constant('root_three_over_two',	(D(3)/D(2)).sqrt())
	print_constant('root_three_over_four',	(D(3)/D(4)).sqrt())
	print_constant('root_three_over_five',	(D(3)/D(5)).sqrt())
	print_constant('pi',					pi())
	print_constant('pi_over_two',			pi() / D(2))
	print_constant('pi_over_three',			pi() / D(3))
	print_constant('pi_over_four',			pi() / D(4))
	print_constant('pi_over_five',			pi() / D(5))
	print_constant('pi_over_six',			pi() / D(6))
	print_constant('two_pi',				tau())
	print_constant('root_pi',				pi().sqrt())
	print_constant('root_pi_over_two',		(pi() / D(2)).sqrt())
	print_constant('root_pi_over_three',	(pi() / D(3)).sqrt())
	print_constant('root_pi_over_four',		(pi() / D(4)).sqrt())
	print_constant('root_pi_over_five',		(pi() / D(5)).sqrt())
	print_constant('root_pi_over_six',		(pi() / D(6)).sqrt())
	print_constant('root_two_pi',			tau().sqrt())
	print_constant('e',						e(1))
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
			break
		write('\t\t#else')
		blit = not blit

	if traits.total_bits > 64 and traits.int_blittable:
		write('\t\t#endif // MUU_HAS_INT{}'.format(traits.total_bits))

	write('\t};')


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
		write('MUU_PUSH_WARNINGS')
		write('MUU_DISABLE_ALL_WARNINGS')
		write('#include <array>')
		write('MUU_POP_WARNINGS')
		write('')
		write('#if MUU_GCC')
		write('\t#pragma GCC system_header')
		write('#endif')
		write('')
		write('#ifndef MUU_HAS_INT256')
		write('\t#define MUU_HAS_INT256 0')
		write('#endif')
		write('')
		write('MUU_PUSH_WARNINGS')
		write('MUU_DISABLE_ALL_WARNINGS')
		write('')
		write('MUU_NAMESPACE_START')
		write('{')
		indent = indent + 1 

		# data tables
		write('template <size_t TotalBits, size_t SignificandBits>')
		write('struct float_test_data_by_traits;')
		write('')

		# (padding_bits, sign_bits, exponent_bits, integer_part_bits, significand_bits)

		write_float_data(file, FloatTraits(  0, 1,  5, 0,  10)) # 16-bit 'half'
		write('')
		write_float_data(file, FloatTraits(  0, 1,  8, 0,  23)) # 32-bit float
		write('')
		write_float_data(file, FloatTraits(  0, 1, 11, 0,  52)) # 64-bit double
		write('')
		write_float_data(file, FloatTraits(  0, 1, 15, 1,  63)) # 80-bit long double
		write('')
		write_float_data(file, FloatTraits( 48, 1, 15, 1,  63)) # 80-bit long double stored as 128 bits
		write('')
		write('#if MUU_HAS_FLOAT128')
		write_float_data(file, FloatTraits(  0, 1, 15, 0, 112)) # 128-bit quad
		write('#endif // MUU_HAS_FLOAT128')
		write('')
		#write_float_data(file, FloatTraits(256,  0, 19, 0, 237))

		# alias by type
		write('template <typename T>')
		write('struct float_test_data : float_test_data_by_traits<sizeof(T) * CHAR_BIT, std::numeric_limits<T>::digits> {};')

		indent = indent - 1 
		write('}')
		write('MUU_NAMESPACE_END')
		write('')
		write('MUU_POP_WARNINGS')
		

if __name__ == '__main__':
	utils.run(main)
