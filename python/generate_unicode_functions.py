#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

# conversions between character types of the same size: https://godbolt.org/z/KeddME

import sys
import os.path as path
import utils
import re
import math
import bisect


#### SETTINGS / MISC ##################################################################################################



class G: # G for Globals
	generate_tests				= True
	hoist_constant_children		= True
	bitmask_expressions			= True
	bitmask_tables				= True
	depth_limit					= 0
	word_size					= 64
	compound_boolean_limit		= 3
	expression_ids				= False



def exid(id):
	if G.expression_ids:
		return f' /* EX {id} */'
	return ''



def make_bitmask_literal(val, bits = 0):
	if not bits:
		bits = 64 if (val >> 32) > 0 else 32
	return "0x{:X}{}".format(val, 'ull' if bits > 32 else 'u')



def make_bitmask_from_indices(indices):
	mask = 0
	for i in indices:
		mask = mask | (1 << i)
	return mask



def make_bitmask_index_test_expression(index, bitmask, index_offset = 0, bits = 0, cast = True):
	if not bits:
		bits = 64 if (bitmask >> 32) > 0 else 32
	suffix = 'ull' if bits >= 64 else 'u'
	s = f'static_cast<uint_least{bits}_t>({index})' if cast else str(index)
	if index_offset != 0:
		s = '({} {} 0x{:X}u)'.format(s, '-' if index_offset < 0 else '+', abs(index_offset))
	return '(1{} << {}) & {}'.format(suffix, s, make_bitmask_literal(bitmask, bits))



def range_first(r):
	if isinstance(r, int):
		return r
	elif isinstance(r, range):
		return r.start
	else:
		return r[0]



def range_last(r):
	if isinstance(r, int):
		return r
	elif isinstance(r, range):
		return r.stop - 1 # wrong for abs(step) != 1 but I don't do that in this script
	else:
		return r[1]



def range_union(first1, last1, first2, last2):
	if last1 < first2-1 or last2 < first1-1:
		return None
	return (min(first1, first2), max(last1, last2))



def range_intersection(first1, last1, first2, last2):
	if last1 < first2 or last2 < first1:
		return None
	return (max(first1, first2), min(last1, last2))



def is_pow2(v):
	return v & (v-1) == 0



def binary_search(elements, value):
	index = bisect.bisect_left(elements, value)
	if index < len(elements) and elements[index] == value:
		return index
	return None



def indent_with_tabs(text, count = 1):
	if count == 0:
		return text
	indent = '\t' * count
	return indent + ('\n' + indent).join(text.split('\n'))



def compound_or(*bools):
	if 'true' in bools:
		return 'true'
	s = ' || '.join(bools)
	if len(bools) > 1:
		s = '({})'.format(s)
	return s



def compound_and(*bools):
	if 'false' in bools:
		return 'false'
	s = ' && '.join(bools)
	if len(bools) > 1:
		s = '({})'.format(s)
	return s



def strip_brackets(s):
	if s.startswith('(') and s.endswith(')'):
		return s[1:-1]
	return s



def wrap_lines(s, sep = '||', wrap_prefix = '\t', assumed_indent = 0):
	elems = [s.strip() for s in s.split(sep)]
	line_len = 0
	s = ''
	wrap_prefix_len = 0
	for c in wrap_prefix:
		wrap_prefix_len += 4 if c == '\t' else 1
	for e in elems:
		if line_len + len(e) + assumed_indent >= 100:
			s += '\n{}{} {}'.format(wrap_prefix, sep, e)
			line_len = len(sep) + len(e) + 1 + wrap_prefix_len
		elif len(s) > 0:
			s += ' {} {}'.format(sep, e)
			line_len += len(sep) + len(e) + 2
		else:
			s = e
			line_len = len(e)
	return s



def ceil(val):
	return int(math.ceil(val))



def calc_child_size(span_size):
	if span_size <= G.word_size:
		return span_size
	elif span_size <= G.word_size * G.word_size:
		return G.word_size
	else:
		return ceil(span_size / float(G.word_size))



def largest(*collections):
	if not collections:
		return None
	result = None
	for c in collections:
		if result is None or len(result) < len(c):
			result = c
	return result



def smallest(*collections):
	if not collections:
		return None
	result = None
	for c in collections:
		if result is None or len(result) < len(c):
			result = c
	return result



def chunks(l, n):
	n = max(1, n)
	return (l[i:i+n] for i in range(0, len(l), n))



#### SPARSE RANGE #####################################################################################################



class SparseRange:

	def __init__(self, *inital_values):
		self.__values = set()
		self.__ranges = []
		self.__count = None
		self.__first = None
		self.__last = None
		for v in inital_values:
			self.add(v)

	def __add_value(self, val):
		if not isinstance(val, int):
			raise Exception('values must be integers')
		self.__values.add(val)

	def __add_collection(self, col):
		for val in col:
			self.__add_value(val)

	def __add_range(self, first, last):
		if (not isinstance(first, int)) or (not isinstance(last, int)):
			raise Exception('ranges must be integral')
		if last < first:
			raise Exception('reverse ranges are not allowed')
		elif first == last:
			self.__add_value(first)
		else:
			self.__ranges.append((first, last))

	def add(self, first, last = None):
		if self.__count is not None:
			raise Exception('finish() has been called')
		if last is None:
			if isinstance(first, range):
				if first.step != 1:
					raise Exception('ranges must be contiguous')
				self.__add_range(first.start, first.stop-1)
			elif isinstance(first, (list, tuple, dict, set)):
				self.__add_collection(first)
			else:
				self.__add_value(first)
		else:
			self.__add_range(first, last)

	def finished(self):
		return self.__count is not None

	def finish(self):
		if self.finished():
			raise Exception('finish() has already been called')

		self.__count = 0
		if len(self.__ranges) == 0 and len(self.__values) == 0:
			return

		# convert sparse values to a list, sort them and convert contiguous spans into ranges
		self.__values = [v for v in self.__values]
		if len(self.__values) > 0:
			self.__values.sort()
			current_range = None
			temp_values = []
			for v in self.__values:
				if current_range is None:
					current_range = [v, v]
				elif v == current_range[1] + 1:
					current_range[1] = v
				else:
					if (current_range[1] > current_range[0]):
						self.__ranges.append((current_range[0], current_range[1]))
					else:
						temp_values.append(current_range[0])
					current_range = [v, v]
			if (current_range[1] > current_range[0]):
				self.__ranges.append((current_range[0], current_range[1]))
			else:
				temp_values.append(current_range[0])
			self.__values = temp_values

		# see if any of the remaining sparse values belong to any of the ranges or can be appended to one
		if len(self.__values) > 0 and len(self.__ranges) > 0:
			temp_values = []
			for v in self.__values:
				matched = False
				for r in range(0, len(self.__ranges)):
					if v >= self.__ranges[r][0] and v <= self.__ranges[r][1]:
						matched = True
						break
					elif v == self.__ranges[r][0] - 1:
						self.__ranges[r] = (v, self.__ranges[r][1])
						matched = True
						break
					elif v == self.__ranges[r][1] + 1:
						self.__ranges[r] = (self.__ranges[r][0], v)
						matched = True
						break
				if not matched:
					temp_values.append(v)
			self.__values = temp_values

		# merge overlapping ranges, remove ranges completely contained by others
		if len(self.__ranges) > 1:
			while True:
				pass_changed = False
				for r1 in range(0, len(self.__ranges)):
					for r2 in range(r1+1, len(self.__ranges)):
						if self.__ranges[r1] is None or self.__ranges[r2] is None:
							continue
						union = range_union(self.__ranges[r1][0], self.__ranges[r1][1], self.__ranges[r2][0], self.__ranges[r2][1])
						if union is not None:
							self.__ranges[r1] = union
							self.__ranges[r2] = None
							pass_changed = True
							break
					if pass_changed:
						break
				if not pass_changed:
					break
			self.__ranges = [r for r in self.__ranges if r is not None]

		# combine the sets of ranges and sparse values into a sorted list
		self.__sparse_value_count = len(self.__values)
		self.__contiguous_subrange_count = len(self.__ranges)
		self.__values = self.__values + self.__ranges
		self.__values.sort(key=range_first)

		# finalize
		self.__ranges = None
		self.__sparse_values = None
		self.__first = range_first(self.__values[0])
		self.__last = range_last(self.__values[-1])
		for v in self.__values:
			self.__count += (range_last(v) - range_first(v)) + 1

	def __len__(self):
		return self.__count if self.__count is not None else 0

	def __bool__(self):
		return self.__count is not None and self.__count > 0

	def __contains__(self, val):
		if not self.finished():
			raise Exception('finish() has not been called')
		if not isinstance(val, int):
			raise Exception('values must be integers')
		if self.__count > 0 and self.__first <= val and self.__last >= val:
			if self.__sparse_values is None:
				self.__sparse_values = [v for v in self]
			return binary_search(self.__sparse_values, val) is not None
		return False

	def stringify(self, formatter = None, joiner = ", "):
		if not self.finished():
			raise Exception('finish() has not been called')
		if formatter is None:
			return joiner.join(str(v) for v in self.__values)
		else:
			s = ""
			for v in self.__values:
				if len(s) > 0:
					s += joiner
				if isinstance(v, int):
					s += formatter(v)
				else:
					s += formatter(v[0]) + " - " + formatter(v[1])
			return s

	def __str__(self):
		return self.stringify()

	class __Iterator:
		def __init__(self, values):
			self.__values = values
			self.__idx = 0
			self.__subidx = 0

		def __iter__(self):
			return self

		def __next__(self):
			if not self.__values or self.__idx >= len(self.__values):
				raise StopIteration
			elem = self.__values[self.__idx]
			if isinstance(elem, tuple):
				val = elem[0] + self.__subidx
				if val == elem[1]:
					self.__idx = self.__idx + 1
					self.__subidx = 0
				else:
					self.__subidx = self.__subidx + 1
				return val
			else:
				self.__idx = self.__idx + 1
				self.__subidx = 0
				return elem

	def __iter__(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__Iterator(self.__values)

	def first(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__first

	def last(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__last

	def contiguous(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__count > 0 and self.__count == (self.__last - self.__first + 1)

	def contiguous_subrange_count(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__contiguous_subrange_count

	def contiguous_subranges(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		for v in self.__values:
			if isinstance(v, tuple):
				yield v

	def sparse_value_count(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__sparse_value_count

	def sparse_values(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		for v in self.__values:
			if not isinstance(v, tuple):
				yield v



#### CODEPOINT CHUNK ##################################################################################################



class CodepointChunk:

	class __Data:
		def __init__(self, level = 0):
			self.range = SparseRange()
			self.level = level
			self.span_first = None
			self.span_last = None

	def __init__(self, code_unit, data=None):
		self.__code_unit = code_unit
		self.__finished = False
		self.__children = None
		self.__expr = None
		self.__expr_handles_low_end = True
		self.__expr_handles_high_end = True
		if data is not None:
			if not isinstance(data, self.__Data):
				raise Exception("nope")
			self.__data = data
			self.__finish()
		else:
			self.__data = self.__Data()

	def range(self):
		return self.__data.range

	def __bool__(self):
		return bool(self.range())

	def __len__(self):
		return len(self.range())

	def first(self):
		return self.range().first()

	def last(self):
		return self.range().last()

	def first_lit(self):
		return self.__code_unit.literal(self.first())

	def last_lit(self):
		return self.__code_unit.literal(self.last())

	def span_first(self):
		return self.__data.span_first

	def span_last(self):
		return self.__data.span_last

	def span_first_lit(self):
		return self.__code_unit.literal(self.span_first())

	def span_last_lit(self):
		return self.__code_unit.literal(self.span_last())

	def span_size(self):
		return (self.span_last() - self.span_first()) + 1

	def level(self):
		return self.__data.level

	def root(self):
		return self.level() == 0

	def always_returns_true(self):
		return self and len(self) == self.span_size()

	def always_returns_false(self):
		return not self

	def has_expression(self):
		return self.__expr is not None

	def makes_bitmask_table(self):
		return (G.bitmask_tables
			and (self.last() - self.first() + 1) >= G.word_size * 4
			and (self.last() - self.first() + 1) <= G.word_size * 256
			and not self.range().contiguous()
			and (len(self) / float(self.last() - self.first() + 1)) >= 0.025
		)

	def child_selector(self):
		s = 'static_cast<uint_least64_t>(c)'
		if (self.first() > 0):
			s = '({} - 0x{:X}ull)'.format(s, self.first())
		return s + ' / 0x{:X}ull'.format(self.__children[0].span_size())

	def expression(self, clamp = False):
		if self.__expr is None:
			return None
		elif not clamp or (self.__expr_handles_low_end and self.__expr_handles_high_end):
			return self.__expr
		else:
			bools = []
			if not self.__expr_handles_low_end:
				bools.append('c >= '+ self.span_first_lit())
			if not self.__expr_handles_high_end:
				bools.append('c <= '+ self.span_last_lit())
			if len(bools) == 0 or not self.always_returns_true():
				bools.append(self.__expr)
			return strip_brackets(compound_and(*bools)) + exid(16)

	def add(self, first, last = None):
		if self.__finished:
			raise Exception('the chunk is read-only')
		self.range().add(first, last)

	def __finish(self):
		if self.__finished:
			return
		if not self.range().finished():
			self.range().finish()
		self.__finished = True
		if self.root():
			self.__data.span_first = self.first()
			self.__data.span_last = self.last()
		if self.range():
			assert self.first() >= self.span_first()
			assert self.last() <= self.span_last()

		# try to figure out a return expression if possible.

		# false
		if self.always_returns_false():
			self.__expr = f'false{exid(0)}'

		# true
		elif self.always_returns_true():
			self.__expr = f'true{exid(1)}'
			self.__expr_handles_low_end = self.span_first() == 0
			self.__expr_handles_high_end = self.span_last() == self.__code_unit.max

		# c != A
		elif (len(self) == self.span_size() - 1):
			gap = None
			for i in range(self.span_first(), self.span_last()+1):
				if i not in self.range():
					gap = i
					break
			assert gap is not None
			self.__expr = 'c != ' + self.__code_unit.literal(gap) + exid(2)
			self.__expr_handles_low_end = gap == self.span_first()
			self.__expr_handles_high_end = gap == self.span_last()

		# c == A
		# c >= A
		# c >= A && c <= B
		elif self.range().contiguous():
			if len(self) == 1:
				self.__expr = 'c == ' + self.first_lit() + exid(3)
			elif (self.first() > self.span_first()) and (self.last() < self.span_last()):
				self.__expr = '(c >= {} && c <= {})'.format(self.first_lit(), self.last_lit()) + exid(4)
			elif self.last() < self.span_last():
				assert self.first() == self.span_first()
				self.__expr = 'c <= ' + self.last_lit() + exid(5)
				self.__expr_handles_low_end = False
			else:
				assert self.first() > self.span_first()
				assert self.last() == self.span_last(), "{} {}".format(self.last(), self.span_last())
				self.__expr = 'c >= ' + self.first_lit() + exid(6)
				self.__expr_handles_high_end = False

		if self.__expr is not None:
			return

		# c % A == 0
		# (c + A) % B == 0
		for div in range(2, 11):
			for add in range(0, div):
				ok = True
				for i in range(self.first(), self.last() + 1):
					if (i + add) % div == 0:
						ok = ok and i in self.range()
					else:
						ok = ok and i not in self.range()
					if not ok:
						break
				if ok:
					s = 'static_cast<uint_least32_t>(c)'
					if (add):
						s = '({} + {}u)'.format(s, add)
					bools = [ '({} % {}u) == 0u'.format(s, div) ]
					self.__expr_handles_low_end = False
					self.__expr_handles_high_end = False
					if (self.last() < self.span_last()):
						bools.insert(0, 'c <= {}'.format(self.last_lit()))
						self.__expr_handles_high_end = True
					if (self.first() > self.span_first()):
						bools.insert(0, 'c >= {}'.format(self.first_lit()))
						self.__expr_handles_low_end = True
					self.__expr = compound_and(*bools) + exid(7)
					break
			if self.__expr:
				break

		if self.__expr is not None:
			return

		# c & A
		if G.bitmask_expressions and (self.last() - self.first() + 1) <= G.word_size:
			bitmask = 0
			for i in self.range():
				shift = i - self.first()
				if shift >= G.word_size:
					break
				bitmask |= 1 << shift
			bools = [ make_bitmask_index_test_expression('c', bitmask, -self.first()) ]
			self.__expr_handles_low_end = False
			self.__expr_handles_high_end = False
			if (self.last() < self.span_last()):
				bools.insert(0, 'c <= {}'.format(self.last_lit()))
				self.__expr_handles_high_end = True
			if (self.first() > self.span_first()):
				bools.insert(0, 'c >= {}'.format(self.first_lit()))
				self.__expr_handles_low_end = True
			self.__expr = wrap_lines(compound_and(*bools), sep='&&', wrap_prefix='\t\t', assumed_indent=self.level()*8)  + exid(8)


		if self.__expr is not None:
			return

		child_first = self.first()
		child_last = self.last()
		child_span = child_last - child_first + 1
		subdivision_allowed = (
			(G.depth_limit <= 0 or (self.level()+1) < G.depth_limit)
			and child_span > 4
			and calc_child_size(child_span) < child_span
		)

		# (c >= A && c <= B) || c == C || c == D ...
		if (self.range().sparse_value_count() + self.range().contiguous_subrange_count()) <= G.compound_boolean_limit or not subdivision_allowed:
			self.__expr_handles_low_end = False
			self.__expr_handles_high_end = False
			bools = []
			for f, l in self.range().contiguous_subranges():
				if l == f + 1:
					if f > 0:
						bools.append('c == {}'.format(self.__code_unit.literal(f)))
					bools.append('c == {}'.format(self.__code_unit.literal(l)))
				else:
					if f > 0:
						bools.append('(c >= {} && c <= {})'.format(self.__code_unit.literal(f), self.__code_unit.literal(l)))
					else:
						bools.append('c <= {}'.format(self.__code_unit.literal(l)))
				self.__expr_handles_low_end = self.__expr_handles_low_end or f == self.span_first()
				self.__expr_handles_high_end = self.__expr_handles_high_end or l == self.span_last()
			for v in self.range().sparse_values():
				bools.append('c == ' + self.__code_unit.literal(v))
				self.__expr_handles_low_end = self.__expr_handles_low_end or v == self.span_first()
				self.__expr_handles_high_end = self.__expr_handles_high_end or v == self.span_last()
			self.__expr = wrap_lines(compound_or(*bools), wrap_prefix='\t\t') + exid(9)


		if self.__expr is not None:
			return

		# haven't been able to make an expression so check if the chunk
		# can be made into a bitmask lookup table
		if self.makes_bitmask_table():
			return

		# couldn't figure out a return expression or make a bitmask lookup table, so subdivide
		child_node_max_size = calc_child_size(child_span)
		child_nodes = ceil(child_span / float(child_node_max_size))
		self.__children = [None] * child_nodes
		for i in self.range():
			relative_value = i - child_first
			assert relative_value >= 0
			child_index = int(relative_value / float(child_node_max_size))
			data = self.__children[child_index]
			if data is None:
				data = self.__Data(self.level() + 1)
				data.span_first = child_first + child_index * child_node_max_size
				data.span_last = min(data.span_first + child_node_max_size - 1, child_last)
				self.__children[child_index] = data
			assert i >= data.span_first
			assert i <= data.span_last
			data.range.add(i)
		for i in range(0, child_nodes):
			if self.__children[i] is not None:
				self.__children[i] = CodepointChunk(self.__code_unit, self.__children[i])
		for child_index in range(0, child_nodes):
			child = self.__children[child_index]
			if child is None:
				data = self.__Data(self.level() + 1)
				data.span_first = child_first + child_index * child_node_max_size
				data.span_last = min(data.span_first + child_node_max_size - 1, child_last)
				self.__children[child_index] = CodepointChunk(self.__code_unit, data)

	def __str__(self):
		self.__finish()
		s = ''
		if self.has_expression():
			return s + 'return {};'.format(strip_brackets(self.expression(self.root())))
		else:
			exclusions = []
			assumptions = []
			if self.first() > 0 and (self.root() or self.first() > self.span_first()):
				exclusions.append('c < ' + self.first_lit())
			else:
				assumptions.append('c >= ' + self.first_lit())
			if self.span_last() < self.__code_unit.max:
				if (self.root() or self.last() < self.span_last()):
					exclusions.append('c > ' + self.last_lit())
				else:
					assumptions.append('c <= ' + self.last_lit())
			if exclusions:
				s += 'if ({})\n\treturn false{};\n'.format(strip_brackets(compound_or(*exclusions)), exid(10))
			if assumptions:
				s += 'MUU_ASSUME({}){};'.format(strip_brackets(compound_and(*assumptions)), exid(11))
				s += '\n'
			if exclusions or assumptions:
				s += '\n'

			summary = "// {} codepoints from {} ranges (spanning a search area of {})".format(
				len(self),
				self.range().sparse_value_count() + self.range().contiguous_subrange_count(),
				self.span_size()
			)

			if (self.makes_bitmask_table()):
				table_name = 'bitmask_table_' + str(self.level())
				s += 'constexpr uint_least{}_t {}[] = \n{{'.format(G.word_size, table_name)
				fmt_str = "\t0x{{:0{}X}}{{}},".format(int(G.word_size/4))
				idx = -1
				for v in range(self.first(), self.last() + 1, G.word_size):
					idx += 1
					if (G.word_size >= 256 or ((idx % int(min(256 / G.word_size, 6))) == 0)):
						s += '\n'
					mask = 0
					for i in range(v, min(v + G.word_size, self.last() + 1)):
						if i in self.range():
							mask = mask | (1 << (i - v))
					s += fmt_str.format(mask, 'u')#'ull' if G.word_size > 32 else 'u')
				element_selector = '(static_cast<uint_least{}_t>(c) - {}) / {}'.format(
					G.word_size,
					make_bitmask_literal(self.first(), G.word_size),
					make_bitmask_literal(G.word_size, G.word_size)
				)
				bit_selector = 'static_cast<uint_least{}_t>(c)'.format(G.word_size)
				if (self.first() % G.word_size != 0):
					bit_selector = '({} - {})'.format(bit_selector, make_bitmask_literal(self.first(), G.word_size))
				bit_selector = '{} % {}'.format(bit_selector, make_bitmask_literal(G.word_size, G.word_size))
				s += '\n};'
				s += '\nreturn {}[{}]\n\t& ({} << ({}));'.format(
					table_name,
					element_selector,
					make_bitmask_literal(1, G.word_size),
					bit_selector
				)
				s += '\n' + summary
				return s

			always_true = []
			always_false = []
			expressions_or_switches = []
			selector_references = 1
			for i in range(0, len(self.__children)):
				if self.__children[i].always_returns_false():
					always_false.append((i,self.__children[i]))
				elif self.__children[i].always_returns_true():
					always_true.append((i,self.__children[i]))
				else:
					expressions_or_switches.append((i,self.__children[i]))

			hoist_constants = G.hoist_constant_children and G.bitmask_expressions
			always_true_selector = None
			if (hoist_constants and 2 <= len(always_true) <= G.word_size):
				always_true_selector = make_bitmask_index_test_expression(
					'@@SELECTOR@@',
					make_bitmask_from_indices([c[0] for c in always_true]),
					0,
					G.word_size,
					False)
				selector_references += 1
				always_true = []

			always_false_selector = None
			if (hoist_constants and 2 <= len(always_false) <= G.word_size):
				always_false_selector = make_bitmask_index_test_expression(
					'@@SELECTOR@@',
					make_bitmask_from_indices([c[0] for c in always_false]),
					0,
					G.word_size,
					False)
				selector_references += 1
				always_false = []

			default = None
			default_check = None
			if (len(always_false) > len(always_true)):
				default = False
				default_check = lambda c: c.always_returns_false()
			elif (always_true and len(always_true) >= len(always_false)):
				default = True
				default_check = lambda c: c.always_returns_true()

			emittables = []
			emittables_simple_equality = []
			emittables_other = []
			emittables_all_have_expressions = True
			defaulted = 0
			for i in range(0, len(self.__children)):
				child = self.__children[i]
				if ((always_true_selector and child.always_returns_true())
					or (always_false_selector and child.always_returns_false())):
					continue
				if (default_check and default_check(child)):
					defaulted += 1
					continue
				emittables.append((i, child))
				emittables_all_have_expressions = emittables_all_have_expressions and child.has_expression()
				if len(child) == 1 and child.has_expression() and not child.always_returns_true():
					emittables_simple_equality.append((i, child))
				else:
					emittables_other.append((i, child))
			if defaulted == 0:
				default = None

			selector = self.child_selector()
			selector_name = 'child_index_{}'.format(self.level())
			if selector_references > 1:
				s += 'const auto {} = {};\n'.format(selector_name, selector)

			requires_switch = len(emittables_other) > 1 or not emittables_all_have_expressions
			return_trues = []
			if always_true_selector:
				return_trues.append(always_true_selector)
			elif always_false_selector and not expressions_or_switches:
				return_trues.append('!({})'.format(always_false_selector))
				always_false_selector = None
			return_trues += [e[1].expression() for e in emittables_simple_equality]
			if not requires_switch:
				return_trues += [e[1].expression() for e in emittables_other if e[1].has_expression()]
			
			return_falses = []
			if always_false_selector:
				return_falses.append(always_false_selector)

			for l, v in [(return_trues, True), (return_falses, False)]:
				if not l:
					continue
				ret = '\n\t|| '.join(l)
				if (return_trues and return_falses) or requires_switch or default is not None:
					s += 'if ({})\n\treturn {}{};\n'.format(ret, 'true' if v else 'false', exid(30))
				else:
					s += 'return {}{}{}{};'.format(
						'' if v else '!(',
						strip_brackets(ret),
						'' if v else ')',
						exid(31)
					)

			if (return_trues and return_falses) or requires_switch or default is not None:
				if len(emittables_other) == 0 and default is not None:
					s += 'return {}{};\n'.format(str(default).lower(), exid(12))
				elif not requires_switch:
					if default is True:
						s += 'return ((@@SELECTOR@@) != {})\n\t|| ({}){};'.format(
							emittables_other[0][0],
							strip_brackets(emittables_other[0][1].expression()),
							exid(13)
						)
					elif default is False:
						s += 'return ((@@SELECTOR@@) == {})\n\t&& ({}){};'.format(
							emittables_other[0][0],
							strip_brackets(emittables_other[0][1].expression()),
							exid(14)
						)
					else:
						selector_references -= 1
						s += 'return {}{};'.format(strip_brackets(emittables_other[0][1].expression()), exid(15))
				else:
					s += "switch (@@SELECTOR@@)\n"
					s += "{\n"
					emitted = 0
					for i, c in emittables_other:
						s += '\tcase 0x{:02X}:{}{}{}'.format(
							i,
							' ' if c.has_expression() else ' // [{}] {:04X} - {:04X}\n\t{{\n'.format(i, c.span_first(), c.span_last()),
							indent_with_tabs(str(c), 0 if c.has_expression() else 2),
							'\n' if c.has_expression() else '\n\t}\n',
						)
						emitted += 1
					s += '\t{};\n'.format('MUU_NO_DEFAULT_CASE' if default is None else 'default: return '+str(default).lower())
					s += "}"
					if (emitted <= 1):
							s += "\n/* FIX ME: switch has only {} case{}! */".format(emitted, 's' if emitted > 1 else '')
					s += '\n' + summary

			if selector_references > 0:
				s = s.replace('@@SELECTOR@@', selector_name if selector_references > 1 else selector)
			return s



#### UNICODE DATABASE ##################################################################################################



class UnicodeDatabase(object):

	__re_code_point = re.compile(r'^([0-9a-fA-F]+);(.+?);([a-zA-Z]+);')

	def __init_code_points(self):
		self.__code_points = SparseRange()
		self.__categories = dict()
		code_point_list = utils.read_all_text_from_file(
			path.join(utils.get_script_folder(), 'Unicode_UnicodeData.txt'),
			'https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt'
		)
		first = -1
		for line in code_point_list.split('\n'):
			match = self.__re_code_point.search(line)
			if not match:
				if (first > -1):
					raise Exception('Previous code point indicated the start of a range but the next one was null')
				continue
			cp = int(f'0x{match.group(1)}', 16)
			last = None
			category_key = None
			if (first > -1):
				last = cp
				category_key = match.group(3)
			else:
				first = cp
				if not match.group(2).endswith(', First>'):
					last = cp
					category_key = match.group(3)

			if last is not None:
				self.__code_points.add(first, last)
				category = self.__categories.get(category_key)
				if category is None:
					category = SparseRange()
					self.__categories[category_key] = category
				category.add(first, last)
				first = -1
		self.__code_points.finish()
		for _, v in self.__categories.items():
			v.finish()
		print(f"Read {len(self.__code_points)} code points from unicode database.")


	__re_property = re.compile(r'^\s*([0-9a-fA-F]{4})\s*(?:\.\.\s*([0-9a-fA-F]{4})\s*)?;\s*([A-Za-z_]+)\s+')
	def __init_properties(self):
		self.__properties = dict()
		property_list = utils.read_all_text_from_file(
			path.join(utils.get_script_folder(), 'Unicode_PropList.txt'),
			'https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt'
		)
		for line in property_list.split('\n'):
			match = self.__re_property.search(line)
			if match:
				first = int(f'0x{match.group(1)}', 16)
				last = match.group(2)
				last = int(f'0x{last}', 16) if last is not None else first
				property_key = match.group(3)
				props = self.__properties.get(property_key)
				if props is None:
					props = SparseRange()
					self.__properties[property_key] = props
				props.add(first, last)
		for _, v in self.__properties.items():
			v.finish()

	def __init__(self):
		self.__init_code_points()
		self.__init_properties()

	def __iter__(self):
		return self.__code_points.__iter__()

	def category(self, cp):
		for k, v in self.__categories.items():
			if cp in v:
				return k
		return None

	def has_property(self, cp, prop):
		p = self.__properties.get(prop)
		if p is not None:
			return cp in p
		return False

	def has_properties(self, cp, props):
		for p in props:
			if not self.has_property(cp, p):
				return False
		return True

	def category_iterator(self, cat):
		for cp in self.__categories[cat]:
			yield cp


__ucd = None
def ucd():
	global __ucd
	if __ucd is None:
		__ucd = UnicodeDatabase()
	return __ucd



__code_point_range_cache = dict()
def get_code_point_range(h):
	assert isinstance(h, int)
	global __code_point_range_cache
	return __code_point_range_cache.get(h)



def store_code_point_range(h, code_points):
	assert isinstance(h, int)
	assert isinstance(code_points, SparseRange)
	global __code_point_range_cache
	__code_point_range_cache[h] = code_points



def get_code_points_in_categories(*categories):
	h = utils.multi_hash(*categories)
	code_points = get_code_point_range(h)
	if code_points is None:
		code_points = SparseRange()
		for cat in categories:
			for cp in ucd().category_iterator(cat):
				code_points.add(cp)
		code_points.finish()
		store_code_point_range(h, code_points)
	return code_points



def get_code_points_with_properties(*properties):
	h = utils.multi_hash(*properties)
	code_points = get_code_point_range(h)
	if code_points is None:
		code_points = SparseRange()
		for cp in ucd():
			if ucd().has_properties(cp, properties):
				code_points.add(cp)
		code_points.finish()
		store_code_point_range(h, code_points)
	return code_points



#### CODE UNIT #########################################################################################################



class CodeUnit(object):

	__types = {
		'char': 			(8,   ''),
		'unsigned char': 	(8,   ''), # unsigned char is used when the compiler doesn't support char8_t
		'char8_t':			(8, 'u8'),
		'char16_t':			(16, 'u'),
		'char32_t':			(32, 'U'),
		'wchar_t':			(16, 'L'), # 16 isn't necessarily true but it doesn't matter here
	}

	__escapes = {
		0x00: '\\0',
		0x07: '\\a',
		0x08: '\\b',
		0x09: '\\t',
		0x0A: '\\n',
		0x0B: '\\v',
		0x0C: '\\f',
		0x0D: '\\r'
	}

	def __init__(self, typename):
		assert isinstance(typename, str)
		assert typename in self.__types

		self.typename = typename
		self.bits = self.__types[typename][0]
		self.max = min(0x10FFFF, (1 << self.bits) - 1)
		self.equivalent_integer = 'uint8_t' if self.bits == 8 else f'uint_least{self.bits}_t'
		self.can_represent_ascii = True
		self.can_represent_any_unicode = self.max >= 128
		self.can_represent_all_unicode = self.max >= 0x10FFFF
		self.literal_prefix = self.__types[typename][1]
		self.integral_literals = self.typename == 'unsigned char'
		self.private_api = self.typename == 'unsigned char'
		self.proxy = self.typename in ('char', 'wchar_t')
		if self.proxy:
			self.proxy_typename = 'wchar_unicode_t' if self.typename == 'wchar_t' else 'char_unicode_t'

	def literal(self, codepoint):
		if self.integral_literals or (self.bits == 8 and codepoint >= 0x80):
			if codepoint > 0xFFFF:
				return f"0x{codepoint:08X}u"
			elif codepoint > 0xFF:
				return f"0x{codepoint:04X}u"
			else:
				return f"0x{codepoint:02X}u"
		else:
			if codepoint < 32:
				esc = self.__escapes.get(codepoint)
				if esc:
					return self.literal_prefix + f"'{esc}'"
			if (32 <= codepoint < 127):
				c = chr(codepoint)
				if c.isprintable():
					return self.literal_prefix + f"'{c}'"
			if codepoint > 0xFFFF:
				return self.literal_prefix + "'\\U{:08X}'".format(codepoint)
			elif codepoint > 0xFF:
				return self.literal_prefix + "'\\u{:04X}'".format(codepoint)
			else:
				return self.literal_prefix + "'\\x{:02X}'".format(codepoint)



#### FUNCTION GENERATORS ###############################################################################################



def write_function_header(file, code_unit, name, return_type, description):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	if not code_unit.private_api:
		write('\t/// \\brief\t\t' + ("\n\t///\t\t\t\t".join(description.split('\n'))))
		write('\t/// \\ingroup\tstrings')
	write('\t[[nodiscard]]')
	if code_unit.proxy:
		write('\tMUU_ALWAYS_INLINE')
	write('\tMUU_ATTR(const)')
	write(f'\tconstexpr {return_type} {name}({code_unit.typename} c) noexcept')
	write('\t{')



def write_function_body(file, text):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	write(indent_with_tabs(text, 2))



def write_function_footer(file):
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	write('\t}')
	write('')



def write_identification_function(file, code_unit, name, description, categories=None,
		characters=None, properties=None, min_codepoint=0, max_codepoint=0x10FFFF):
	assert file is not None
	assert isinstance(code_unit, CodeUnit)
	assert isinstance(name, str)
	assert isinstance(description, str)
	assert categories is not None or characters is not None or properties is not None

	if code_unit.proxy:
		write_function_header(file, code_unit, name, 'bool', description)
		write_function_body(file, f'using namespace impl;')
		write_function_body(file, f'return {name}(static_cast<{code_unit.proxy_typename}>(c));')
		write_function_footer(file)
		return

	min_codepoint = max(min_codepoint, 0)
	max_codepoint = min(max_codepoint, 0x10FFFF)
	local_min = min_codepoint
	local_max = min(code_unit.max, max_codepoint)
	if categories is None:
		categories = False
	elif not utils.is_collection(categories):
		categories = (categories,)
	if characters is None:
		characters = False
	elif not utils.is_collection(characters):
		characters = (characters,)
	if properties is None:
		properties = False
	elif not utils.is_collection(properties):
		properties = (properties,)

	code_points_hash = utils.multi_hash(local_min, local_max, categories, characters, properties)
	code_points = get_code_point_range(code_points_hash)
	if code_points is None:
		code_points = SparseRange()
		if categories:
			for cp in get_code_points_in_categories(*categories):
				if cp < local_min:
					continue
				if cp > local_max:
					break
				code_points.add(cp)

		if characters:
			for c in characters:
				cp = c
				if isinstance(cp, str):
					cp = ord(cp)
				if isinstance(cp, int):
					if local_min <= cp <= local_max:
						code_points.add(cp)
				elif isinstance(cp, tuple) and len(cp) == 2:
					cp = range_intersection(
						ord(cp[0]) if isinstance(c[0], str) else cp[0],
						ord(cp[1]) if isinstance(c[1], str) else cp[1],
						local_min,
						local_max
					)
					if cp is not None:
						code_points.add(cp[0], cp[1])
				else:
					raise Exception("Invalid argument")

		if properties:
			for cp in get_code_points_with_properties(*properties):
				if cp < local_min:
					continue
				if cp > local_max:
					break
				code_points.add(cp)

		code_points.finish()
		store_code_point_range(code_points_hash, code_points)

	chunk = None
	if code_points:
		chunk = CodepointChunk(code_unit)
		for (f,l) in code_points.contiguous_subranges():
			chunk.add(f,l)
		for cp in code_points.sparse_values():
			chunk.add(cp)

	write_function_header(file, code_unit, name, 'bool', description)
	if chunk is not None:
		write_function_body(file, str(chunk))
	else:
		write_function_body(file, '(void)c;\nreturn false;')
	write_function_footer(file)



def write_compound_boolean_function(file, code_unit, name, description, *functions):
	assert file is not None
	assert isinstance(code_unit, CodeUnit)
	assert isinstance(name, str)
	assert isinstance(description, str)
	assert functions is not None

	write_function_header(file, code_unit, name, 'bool', description)
	if code_unit.proxy:
		write_function_body(file, f'using namespace impl;')
		write_function_body(file, f'return {name}(static_cast<{code_unit.proxy_typename}>(c));')
	else:
		write_function_body(file, f'return {strip_brackets(compound_or(*[f+"(c)" for f in functions]))};')
	write_function_footer(file)



#### HEADER GENERATOR ##################################################################################################


def write_header(folder, code_unit):
	assert isinstance(folder, str)
	assert isinstance(code_unit, CodeUnit)

	file_path = re.sub(r'\s+', '_', code_unit.typename)
	file_path = path.join(folder, f'unicode_{file_path}.h')
	print("Writing to {}".format(file_path))
	with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
		
		# preamble
		write = lambda txt,end='\n': print(txt, file=file, end=end)
		write('// This file is a part of muu and is subject to the the terms of the MIT license.')
		write('// Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>')
		write('// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		write('// SPDX-License-Identifier: MIT')
		write('//-----')
		write('// this file was generated by generate_unicode_functions.py - do not modify it directly')
		write('')
		write('/// \\file')
		write(f'/// \\attention These are not the droids you are looking for. Try \\ref strings instead.')
		write('')
		write('#pragma once')
		if code_unit.typename == 'char':
			write('#ifdef __cpp_char8_t')
			write('\t#include "../../muu/impl/unicode_char8_t.h"')
			write('#else')
			write('\t#include "../../muu/impl/unicode_unsigned_char.h"')
			write('#endif')
		elif code_unit.typename == 'wchar_t':
			write('#include "../../muu/preprocessor.h"')
			write('#include MUU_MAKE_STRING_2(MUU_CONCAT(MUU_CONCAT(../../muu/impl/unicode_char, MUU_WCHAR_BITS), _t.h))')
		else:
			write('#include "../../muu/fwd.h"')
		write('')
		write('MUU_{}NAMESPACE_START'.format('IMPL_' if code_unit.private_api else ''))
		write('{')

		specifier = None
		if code_unit.typename == 'char':
			specifier = 'character'
		elif code_unit.typename == 'wchar_t':
			specifier = 'wide character'
		elif code_unit.bits == 32:
			specifier = 'UTF code point'
		else:
			specifier = f'UTF-{code_unit.bits} code unit'

		write_identification_function(file, code_unit,
			'is_ascii_whitespace',
			f'Returns true if a {specifier} is whitespace from the ASCII range.',
			properties='White_Space',
			max_codepoint=127
		)
		write_identification_function(file, code_unit,
			'is_non_ascii_whitespace',
			f'Returns true if a {specifier} is whitespace from outside the ASCII range.',
			properties='White_Space',
			min_codepoint=128
		)
		write_compound_boolean_function(file, code_unit,
			'is_whitespace',
			f'Returns true if a {specifier} is whitespace.',
			'is_ascii_whitespace', 'is_non_ascii_whitespace'
		)
		write_compound_boolean_function(file, code_unit,
			'is_not_whitespace',
			f'Returns true if a {specifier} is not whitespace.',
			'!is_whitespace'
		)
		write_identification_function(file, code_unit,
			'is_ascii_letter',
			f"Returns true if a {specifier} is a letter from the ASCII range.",
			categories=('Ll', 'Lm', 'Lo', 'Lt', 'Lu'),
			max_codepoint=127
		)
		write_identification_function(file, code_unit,
			'is_non_ascii_letter',
			f'Returns true if a {specifier} is a letter from outside the ASCII range.',
			categories=('Ll', 'Lm', 'Lo', 'Lt', 'Lu'),
			min_codepoint=128
		)
		write_compound_boolean_function(file, code_unit,
			'is_letter',
			f'Returns true if a {specifier} is a letter.',
			'is_ascii_letter', 'is_non_ascii_letter'
		)
		write_identification_function(file, code_unit,
			'is_ascii_number',
			f"Returns true if a {specifier} is a number from the ASCII range.",
			categories=('Nd', 'Nl'),
			max_codepoint=127
		)
		write_identification_function(file, code_unit,
			'is_non_ascii_number',
			f"Returns true if a {specifier} is a number from outside the ASCII range.",
			categories=('Nd', 'Nl'),
			min_codepoint=128
		)
		write_compound_boolean_function(file, code_unit,
			'is_number',
			f'Returns true if a {specifier} is a number.',
			'is_ascii_number', 'is_non_ascii_number'
		)
		write_identification_function(file, code_unit,
			'is_combining_mark',
			f"Returns true if a {specifier} is a combining mark.",
			categories=('Mn', 'Mc')
		)
		write_identification_function(file, code_unit,
			'is_octal_digit',
			f"Returns true if a {specifier} is an octal digit.",
			characters=(('0', '7'), )
		)
		write_identification_function(file, code_unit,
			'is_decimal_digit',
			f"Returns true if a {specifier} is a decimal digit.",
			characters=(('0', '9'), )
		)
		write_identification_function(file, code_unit,
			'is_hexadecimal_digit',
			f"Returns true if a {specifier} is a hexadecimal digit.",
			characters=(('a', 'f'), ('A', 'F'), ('0', '9'))
		)
		write_function_header(file, code_unit,
			'is_code_point_boundary',
			'bool',
			f"Returns true if a {specifier} is a code point boundary."
		)
		if code_unit.proxy:
			write_function_body(file, f'using namespace impl;')
			write_function_body(file, f'return is_code_point_boundary(static_cast<{code_unit.proxy_typename}>(c));')
		elif code_unit.bits == 32:
			write_function_body(file, f'(void)c;')
			write_function_body(file, 'return true;')
		elif code_unit.bits == 16:
			write_function_body(file, 'return c <= 0xDBFFu || c >= 0xE000u;')
		elif code_unit.bits == 8:
			write_function_body(file, 'return (c & 0b11000000u) != 0b10000000u;')
		write_function_footer(file)

		write('}')
		write('MUU_{}NAMESPACE_END'.format('IMPL_' if code_unit.private_api else ''))



#### MAIN ##############################################################################################################



def main():
	folder = path.join(utils.get_script_folder(), '..', 'include', 'muu', 'impl')

	ucd() # force generation first

	#G.subdivision_allowed = False
	#G.word_size = 32
	#G.compound_boolean_limit = 3
	write_header(folder, CodeUnit('char'))
	write_header(folder, CodeUnit('unsigned char'))
	write_header(folder, CodeUnit('char8_t'))
	write_header(folder, CodeUnit('char16_t'))
	write_header(folder, CodeUnit('char32_t'))
	write_header(folder, CodeUnit('wchar_t'))


if __name__ == '__main__':
	utils.run(main)
