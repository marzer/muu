#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

import utils
import sys
import math
import random
import decimal
from pathlib import Path



__debugging = False
def dprint(*args):
	global __debugging
	if __debugging:
		print(*args)



__pi_multiples = dict()
def pi_multiples():
	global __pi_multiples
	result = __pi_multiples.get(decimal.getcontext().prec)
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
		twopi = s * decimal.Decimal(2)
		threepi = s * decimal.Decimal(3)
		fourpi = s * decimal.Decimal(4)
		decimal.getcontext().prec -= 4
		result = (decimal.Decimal(s), decimal.Decimal(twopi), decimal.Decimal(threepi), decimal.Decimal(fourpi))
		__pi_multiples[decimal.getcontext().prec] = result
	return result



def pi():
	return pi_multiples()[0]



def two_pi():
	return pi_multiples()[1]



def three_pi():
	return pi_multiples()[2]



__phi = dict()
def phi():
	global __phi
	result = __phi.get(decimal.getcontext().prec)
	if result is None:
		decimal.getcontext().prec += 4
		result = (decimal.Decimal(1) + decimal.Decimal(5).sqrt()) / decimal.Decimal(2)
		decimal.getcontext().prec -= 4
		result = +result
		__phi[decimal.getcontext().prec] = result
	return result



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



def cos(x):
	decimal.getcontext().prec += 2
	i, lasts, s, fact, num, sign = 0, 0, 1, 1, 1, 1
	while s != lasts:
		lasts = s
		i += 2
		fact *= i * (i-1)
		num *= x * x
		sign *= -1
		s += num / fact * sign
	decimal.getcontext().prec -= 2
	return decimal.Decimal(s)



def sin(x):
	decimal.getcontext().prec += 2
	i, lasts, s, fact, num, sign = 1, 0, x, 1, x, 1
	while s != lasts:
		lasts = s
		i += 2
		fact *= i * (i-1)
		num *= x * x
		sign *= -1
		s += num / fact * sign
	decimal.getcontext().prec -= 2
	return decimal.Decimal(s)



def tan(x):
	decimal.getcontext().prec += 2
	s = sin(x) / cos(x);
	decimal.getcontext().prec -= 2
	return decimal.Decimal(s)



def __try_quantize(d, exp, rounding=None):
	try:
		return d.quantize(exp, rounding=rounding)
	except decimal.InvalidOperation:
		return None

def try_quantize(d, exp, rounding=None):
	prec = decimal.getcontext().prec
	val = __try_quantize(d,exp,rounding=rounding)
	while val is None and decimal.getcontext().prec > 1:
		decimal.getcontext().prec = decimal.getcontext().prec - 1
		val = __try_quantize(d,exp,rounding=rounding)
	decimal.getcontext().prec = prec
	if val is None:
		val = d
	return val


def int_literal(val, bits, always_hex = False):
	if not utils.is_pow2(bits):
		bits = utils.next_pow2(bits)
	if bits > 64:
		vals = []
		for _ in range(0, int(bits / 64)):
			vals.insert(0, int_literal(val & 0xFFFFFFFFFFFFFFFF, 64, always_hex))
			val = val >> 64
		return 'bit_pack({})'.format(', '.join(vals))
	else:
		mask = int((1 << bits) - 1)
		if bits > 32 or always_hex:
			return '0x{:0{}X}_u{}'.format(val & mask, int(bits / 4), bits)
		else:
			return '0b{:0{}b}_u{}'.format(val & mask, bits, bits)



def round_binary(val, places):
	assert len(val) > places
	D = decimal.Decimal

	high = int(val[:places], 2)
	low = D()
	for i in range(places, len(val)):
		n = i - places
		if val[i] == '1':
			low = low + D(1) * (D(2) ** D(-n))
	low.normalize()

	result = ''
	if low == (D(1)/D(2)):
		if high % 2 == 0:
			result = bin(high)
		else:
			result = bin(high+1)
	elif low > (D(1)/D(2)):
		result = bin(high+1)
	else:
		result = bin(high)
	result = result[2:]
	if len(result) < places:
		result = ('0' * (places - len(result))) + result
	return result



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

		self.exponent_bias = (1 << (self.exponent_bits-1)) - 1

	def constant(self, val, allow_blitting = True):
		if self.int_blittable and allow_blitting:
			return int_literal(val, self.total_bits)
		else:
			elem_bits = self.total_bits
			if not utils.is_pow2(elem_bits):
				elem_bits = utils.next_pow2(elem_bits) >> 1
			while (self.total_bits % elem_bits) != 0 or elem_bits > 64:
				elem_bits = elem_bits >> 1
			elems = []
			for _ in range(0, int(self.total_bits / elem_bits)):
				elems.append(int_literal(val, elem_bits, always_hex = True))
				val = val >> elem_bits
			return 'std::array{{ {} }}'.format(', '.join(elems))

	def bit_representation(self,dec):
		D = decimal.Decimal
		#rounded = lambda d,p=self.max_digits10+10: try_quantize(d, D(10) ** -p)
		#dec = rounded(dec).normalize()
		dec = dec.normalize()
		if dec.is_zero():
			return '0' * self.total_bits
		
		#return ''.join('{:0>8b}'.format(c) for c in struct.pack('>e', float(dec)))

		sign_bit = '1' if dec < D(0) else '0'
		if dec < D(0):
			dec = -dec
		assert dec >= D(0)
		dprint('----------------')
		dprint(f'value:           {dec}')
		dprint('sign_bit:        ' + sign_bit)
		
		integral = int(dec.to_integral_value(rounding=decimal.ROUND_FLOOR))
		fractional = dec
		if integral > 0:
			fractional = fractional - D(integral)
			if fractional < D(0):
				fractional = -fractional
		assert integral >= 0
		assert fractional >= D(0)
		assert fractional < D(1)
		assert (integral + fractional) == dec, f"{integral}.{fractional} == {dec}"
		dprint(f'integral:        {integral}')
		dprint(f'fractional:      {fractional}')
		
		integral_bits = ''
		if integral > 0:
			integral_bits = bin(integral)[2:]
		
		float_exponent_offset = 0
		fractional_bits = ''
		prev_prec = decimal.getcontext().prec
		p = prev_prec
		if fractional.is_zero():
			fractional_bits = '0'
		else:
			f = (+fractional).normalize()
			while p > 0:
				decimal.getcontext().prec = p
				f = (f * D(2)).normalize()
				if f >= D(1):
					fractional_bits = fractional_bits + '1'
					if f == D(1):
						break
					f = f - D(1)
				else:
					fractional_bits = fractional_bits + '0'
				p = p - 1
			if integral == 0:
				while fractional_bits[0] == '0':
					fractional_bits = fractional_bits[1:]
					float_exponent_offset = float_exponent_offset - 1
		decimal.getcontext().prec = prev_prec

		dprint('integral_bits:   ' + integral_bits)
		dprint('fractional_bits: ' + fractional_bits)
		
		if (len(integral_bits) + len(fractional_bits)) > (self.significand_bits + 1):
			if len(integral_bits) > (self.significand_bits + 1):
				raise Exception("eh")
			fractional_bits = round_binary(fractional_bits, self.significand_bits + 1 - len(integral_bits))
		
		mantissa_bits = (integral_bits + fractional_bits)#[:self.significand_bits]
		if self.integer_part_bits == 0 and mantissa_bits[0] == '1':
			mantissa_bits = mantissa_bits[1:]
		if len(mantissa_bits) < self.significand_bits:
			mantissa_bits = mantissa_bits + '0' * (self.significand_bits - len(mantissa_bits))
		dprint('mantissa_bits:   ' + mantissa_bits)
		
		exponent = None
		if integral > 0:
			exponent = len(integral_bits) - 1
		if not fractional.is_zero():
			idx = fractional_bits.find('1')
			assert idx >= 0
			if exponent is None:
				exponent = -idx - 1 + float_exponent_offset
		if exponent is None:
			exponent = 0
		dprint(f'exponent:        {exponent}')
		
		exponent_bits = ''
		exponent = exponent + self.exponent_bias
		bit = 1 << (self.exponent_bits-1)
		while bit:
			if bit & exponent:
				exponent_bits = exponent_bits + '1'
			else:
				exponent_bits = exponent_bits + '0'
			bit = bit >> 1
		dprint('exponent_bits:   ' + exponent_bits)
		
		return (self.padding_bits * '0') + sign_bit + "'" + exponent_bits + "'" + mantissa_bits



def write_float_data(file, traits):
	write = lambda txt,end='\n': print(txt, file=file, end=end)

	decimal.getcontext().prec = 256
	decimal.getcontext().rounding = decimal.ROUND_HALF_EVEN
	D = decimal.Decimal
	rounded = lambda d,p=traits.max_digits10: try_quantize(d, D(10) ** -p)

	write('\ttemplate <>')
	write('\tstruct float_test_data_by_traits<{}, {}>'.format(traits.total_bits, traits.digits))
	write('\t{')

	type = 'long double'
	suffix = 'L'
	if traits.total_bits == 128 and traits.significand_bits == 112:
		type = 'float128_t'
		suffix = 'q'

	####
	#### accumulator data table ###############################
	####
	write(f'\t\tstatic constexpr {type} values[] = ')
	write('\t\t{',end='')
	per_line = max(int(108 / (traits.digits10 + 5)), 1)
	sum = D(0)
	min_ = D(10000000000000)
	max_ = D(-10000000000000)
	random.seed(4815162342)
	denom = 10 ** traits.digits10
	denom = (denom, D(denom * 2))
	for i in range(0, 400):
		val = rounded(D(random.randrange(1, denom[0])) / denom[1], p=traits.digits10)
		sum = sum + val
		min_ = min(min_, val)
		max_ = max(max_, val)
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
	write(f'\t\tstatic constexpr {type} values_sum_low         = {rounded(sum * (D(1) - delta), p=traits.digits10)}{suffix};')
	write(f'\t\tstatic constexpr {type} values_sum             = {rounded(sum, p=traits.digits10)}{suffix};')
	write(f'\t\tstatic constexpr {type} values_sum_high        = {rounded(sum * (D(1) + delta), p=traits.digits10)}{suffix};')
	write(f'\t\tstatic constexpr {type} values_min             = {rounded(min_, p=traits.digits10)}{suffix};')
	write(f'\t\tstatic constexpr {type} values_max             = {rounded(max_, p=traits.digits10)}{suffix};')

	####
	#### constants ###############################
	####
	write('')
	constant_inputs = [
		#(D(1), 'one'),
		(D(2), 'two'),
		(D(3), 'three'),
		(pi(), 'pi'),
		(two_pi(), 'two_pi'),
		(three_pi(), 'three_pi'),
		(e(1), 'e'),
		(phi(), 'phi')
	]
	constants = dict()
	constants_skip_list = [
		'one', 'two', 'three', 'four', 'five', 'six',
		'three_pi_over_three', 'three_pi_over_six', 'three_pi_over_seven', 'three_pi_over_eight',
		'e_over_seven', 'e_over_eight',
		'phi_over_seven', 'phi_over_eight'
	]
	print_constant_ = lambda name, value: \
		write(f'\t\tstatic constexpr {type} {name}{" " * (23 - len(name))}= {rounded(value)}{suffix};{" // "+traits.bit_representation(value) if traits.total_bits == 16 else ""}')
	print_constant =  lambda n,v: (print_constant_(n, v), ) if n not in constants_skip_list else None
	for val, name in constant_inputs:
		print_constant(name, val)
		print_constant(f'one_over_{name}', D(1) / val)
		if val != two_pi():
			if val != D(2):
				print_constant(f'{name}_over_two', val / D(2))
			if val != D(3):
				print_constant(f'{name}_over_three', val / D(3))
			if val != D(4) and val != D(2):
				print_constant(f'{name}_over_four', val / D(4))
			if val != D(5):
				print_constant(f'{name}_over_five', val / D(5))
			if val != D(6) and val != D(3) and val != D(2):
				print_constant(f'{name}_over_six', val / D(6))
			if val != D(7) and val != D(2) and val != D(3):
				print_constant(f'{name}_over_seven', val / D(7))
			if val != D(8) and val != D(2) and val != D(3):
				print_constant(f'{name}_over_eight', val / D(8))

		if val != D(1):
			print_constant(f'sqrt_{name}', val.sqrt())
			print_constant(f'one_over_sqrt_{name}', D(1) / val.sqrt())
	print_constant("degrees_to_radians", pi() / D(180))
	print_constant("radians_to_degrees", D(180) / pi())

	####
	#### limits ###############################
	####
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

	####
	#### tangent table ###############################
	####
	#if traits.total_bits >= 64 and type in ('long double', 'float128_t'):
	#	write('')
	#	decimal.getcontext().rounding = decimal.ROUND_FLOOR
	#	tangent_table_max = 16;
	#	write(f'\t\tstatic constexpr std::pair<{type}, {type}> tangents[] =');
	#	write('\t\t{\n\t\t\t', end='')
	#	for i in range(0,tangent_table_max+1):
	#		in_val = -pi() + two_pi() * (D(i) / D(tangent_table_max))
	#		if (in_val.is_infinite()):
	#			continue
	#		if i > 0:
	#			write(',\n\t\t\t', end='')
	#		out_val = tan(in_val)
	#		write(f'{{ {rounded(in_val)}{suffix}, ', end='')
	#		if (out_val >= D(10000000000000000000000)):
	#			write(f'{"1.18973149535723176508575932662800702e+4932q" if type == "float128_t" else "LDBL_MAX"}', end='')
	#		else:
	#			write(f'{rounded(out_val)}{suffix}', end='')
	#		write(' }', end='')
	#	write('\n\t\t};')
	#	decimal.getcontext().rounding = decimal.ROUND_HALF_EVEN

	write('\t};')


def main():
	
	file_path = Path(utils.entry_script_dir(), '..', 'tests', 'float_test_data.h').resolve()
	print("Writing to {}".format(file_path))
	with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
		indent = 0
		write = lambda txt: print(('\t' * indent) + txt if txt != '' else '', file=file)

		# preamble
		write('// This file is a part of muu and is subject to the the terms of the MIT license.')
		write('// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>')
		write('// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		write('// SPDX-License-Identifier: MIT')
		write('//-----')
		write('// this file was generated by generate_float_tests_header.py - do not modify it directly')
		write('')
		write('#pragma once')
		write('#include "settings.h"')
		write('#include "../include/muu/core.h"')
		write('#include "../include/muu/bit.h"')
		write('MUU_DISABLE_WARNINGS;')
		write('#include <array>')
		write('MUU_ENABLE_WARNINGS;')
		write('')
		write('#if MUU_GCC && MUU_HAS_FLOAT128')
		write('\t#pragma GCC system_header')
		write('#endif')
		write('')
		write('#ifndef MUU_HAS_INT256')
		write('\t#define MUU_HAS_INT256 0')
		write('#endif')
		write('')
		write('namespace muu')
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
		write('struct float_test_data : float_test_data_by_traits<sizeof(T) * CHAR_BIT, constants<T>::significand_digits> {};')

		indent = indent - 1 
		write('}')
		

if __name__ == '__main__':
	utils.run(main)
