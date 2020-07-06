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


def write_float_data(file, precision):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	D = decimal.Decimal
	rounded = lambda d,p=precision: d.quantize(D(10) ** -p)

	write('template <>')
	write('struct float_test_data_by_precision<{}>'.format(precision))
	write('{')

	write('\tstatic constexpr long double values[] = '.format(precision))
	write('\t{',end='')
	per_line = int(112 / (precision + 5))
	decimal.getcontext().prec = 120
	sum = D(0)
	for i in range(0, 400):
		numerator = random.randrange(500, 5000)
		val = rounded((D(1) / D(numerator * 2)) * D(random.randrange(1, numerator)))
		sum = sum + val
		if i > 0:
			write(', ',end='')
		if i % per_line == 0:
			write('\n\t\t',end='')
		write('{}L'.format(val), end='')
	write('\n\t};')
	write('')


	delta = (D(10) ** -(max(int(round(3.0*precision/4.0))+1,2)))
	# write('\t// {}'.format(D(1) - delta));
	# write('\t// {}'.format(D(1) + delta));
	write('\tstatic constexpr long double values_sum_low  = {}L;'.format(rounded(sum * (D(1) - delta))))
	write('\tstatic constexpr long double values_sum      = {}L;'.format(rounded(sum)))
	write('\tstatic constexpr long double values_sum_high = {}L;'.format(rounded(sum * (D(1) + delta))))

	write('};')
	write('')


def main():
	
	file_path = path.join(utils.get_script_folder(), '..', 'tests', 'float_test_data.h')
	print("Writing to {}".format(file_path))
	with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
		write = lambda txt: print(txt, file=file)

		# preamble
		write('// This file is a part of muu and is subject to the the terms of the MIT license.')
		write('// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>')
		write('// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		write('// SPDX-License-Identifier: MIT')
		write('//-----')
		write('// this file was generated by generate_float_test_data.py - do not modify it directly')
		write('')
		write('#pragma once')
		write('#include <limits>')
		write('')

		# data tables
		write('template <size_t digits10>')
		write('struct float_test_data_by_precision;')
		write('')
		write_float_data(file, 3)	# digits10 for 16-bit floats
		write_float_data(file, 6)	# digits10 for 32-bit floats
		write_float_data(file, 15)	# digits10 for 64-bit floats
		write_float_data(file, 18)	# digits10 for 80-bit floats (x86 extended)
		write_float_data(file, 33)	# digits10 for 128-bit floats

		# alias by type
		write('template <typename T>')
		write('struct float_test_data : float_test_data_by_precision<std::numeric_limits<T>::digits10> {};')
		

if __name__ == '__main__':
	utils.run(main)
