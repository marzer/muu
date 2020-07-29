#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

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



def make_literal(codepoint):
	if (32 <= codepoint < 127 and chr(codepoint).isprintable()):
		return "U'{}'".format(chr(codepoint))
	elif (codepoint > 0xFFFF):
		return "U'\\U{:08X}'".format(codepoint)
	else:
		return "U'\\u{:04X}'".format(codepoint)



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
	s = 'static_cast<ui{}>({})'.format(bits, index) if cast else str(index)
	if index_offset != 0:
		s = '({} {} 0x{:X}{})'.format(s, '-' if index_offset < 0 else '+', abs(index_offset), suffix)
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

	def __init__(self, data=None):
		self.__finished = False
		self.__children = None
		self.__expr = None
		self.__expr_handles_low_end = True
		self.__expr_handles_high_end = True
		self.__uint_typedefs = set()
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
		return make_literal(self.first())

	def last_lit(self):
		return make_literal(self.last())

	def span_first(self):
		return self.__data.span_first

	def span_last(self):
		return self.__data.span_last

	def span_first_lit(self):
		return make_literal(self.span_first())

	def span_last_lit(self):
		return make_literal(self.span_last())

	def span_size(self):
		return (self.span_last() - self.span_first()) + 1

	def required_uint_typedefs(self):
		return iter(self.__uint_typedefs)

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
		self.__uint_typedefs.add(64)
		s = 'static_cast<ui64>(cp)'
		if (self.first() > 0):
			s = '({} - 0x{:X}ull)'.format(s, self.first())
		return s + ' / 0x{:X}ull'.format(self.__children[0].span_size())

	def expression(self, clamp = False):
		if self.__expr is None:
			return None
		if not clamp or (self.__expr_handles_low_end and self.__expr_handles_high_end):
			return self.__expr
		return '{}{}{}'.format(
			'cp >= {} && '.format(self.span_first_lit()) if not self.__expr_handles_low_end else '',
			'cp <= {} && '.format(self.span_last_lit()) if not self.__expr_handles_high_end else '',
			self.__expr
		)

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
			self.__expr = 'false'

		# true
		elif self.always_returns_true():
			self.__expr = 'true'
			self.__expr_handles_low_end = False
			self.__expr_handles_high_end = False

		# cp != A
		elif (len(self) == self.span_size() - 1):
			gap = None
			for i in range(self.span_first(), self.span_last()+1):
				if i not in self.range():
					gap = i
					break
			assert gap is not None
			self.__expr = 'cp != ' + make_literal(gap)
			self.__expr_handles_low_end = gap == self.span_first()
			self.__expr_handles_high_end = gap == self.span_last()

		# cp == A
		# cp >= A
		# cp >= A && cp <= B
		elif self.range().contiguous():
			if len(self) == 1:
				self.__expr = 'cp == ' + self.first_lit()
			elif (self.first() > self.span_first()) and (self.last() < self.span_last()):
				self.__expr = '(cp >= {} && cp <= {})'.format(self.first_lit(), self.last_lit())
			elif self.last() < self.span_last():
				assert self.first() == self.span_first()
				self.__expr = 'cp <= ' + self.last_lit()
				self.__expr_handles_low_end = False
			else:
				assert self.first() > self.span_first()
				assert self.last() == self.span_last(), "{} {}".format(self.last(), self.span_last())
				self.__expr = 'cp >= ' + self.first_lit()
				self.__expr_handles_high_end = False

		if self.__expr is not None:
			return

		# cp % A == 0
		# (cp + A) % B == 0
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
					s = 'static_cast<ui32>(cp)'
					self.__uint_typedefs.add(32)
					if (add):
						s = '({} + {}u)'.format(s, add)
					bools = [ '({} % {}u) == 0u'.format(s, div) ]
					self.__expr_handles_low_end = False
					self.__expr_handles_high_end = False
					if (self.last() < self.span_last()):
						bools.insert(0, 'cp <= {}'.format(self.last_lit()))
						self.__expr_handles_high_end = True
					if (self.first() > self.span_first()):
						bools.insert(0, 'cp >= {}'.format(self.first_lit()))
						self.__expr_handles_low_end = True
					self.__expr = compound_and(*bools)
					break
			if self.__expr:
				break

		if self.__expr is not None:
			return

		# cp & A
		if G.bitmask_expressions and (self.last() - self.first() + 1) <= G.word_size:
			bitmask = 0
			for i in self.range():
				shift = i - self.first()
				if shift >= G.word_size:
					break
				bitmask |= 1 << shift
			bools = [ make_bitmask_index_test_expression('cp', bitmask, -self.first()) ]
			self.__uint_typedefs.add(64 if bitmask > 0xFFFFFFFF else 32)
			self.__expr_handles_low_end = False
			self.__expr_handles_high_end = False
			if (self.last() < self.span_last()):
				bools.insert(0, 'cp <= {}'.format(self.last_lit()))
				self.__expr_handles_high_end = True
			if (self.first() > self.span_first()):
				bools.insert(0, 'cp >= {}'.format(self.first_lit()))
				self.__expr_handles_low_end = True
			self.__expr = wrap_lines(compound_and(*bools), sep='&&', wrap_prefix='\t\t', assumed_indent=self.level()*8)


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

		# (cp >= A && cp <= B) || cp == C || cp == D ...
		if (self.range().sparse_value_count() + self.range().contiguous_subrange_count()) <= 3 or not subdivision_allowed:
			self.__expr_handles_low_end = False
			self.__expr_handles_high_end = False
			bools = []
			for f, l in self.range().contiguous_subranges():
				if l == f + 1:
					if f > 0:
						bools.append('cp == {}'.format(make_literal(f)))
					bools.append('cp == {}'.format(make_literal(l)))
				else:
					if f > 0:
						bools.append('(cp >= {} && cp <= {})'.format(make_literal(f), make_literal(l)))
					else:
						bools.append('cp <= {}'.format(make_literal(l)))
				self.__expr_handles_low_end = self.__expr_handles_low_end or f == self.span_first()
				self.__expr_handles_high_end = self.__expr_handles_high_end or l == self.span_last()
			for v in self.range().sparse_values():
				bools.append('cp == ' + make_literal(v))
				self.__expr_handles_low_end = self.__expr_handles_low_end or v == self.span_first()
				self.__expr_handles_high_end = self.__expr_handles_high_end or v == self.span_last()
			self.__expr = wrap_lines(compound_or(*bools), wrap_prefix='\t\t')


		if self.__expr is not None:
			return

		# haven't been able to make an expression so check if the chunk
		# can be made into a bitmask lookup table
		if self.makes_bitmask_table():
			self.__uint_typedefs.add(G.word_size)
			return

		# couldn't figure out a return expression or make a bitmask lookup table, so subdivide
		self.__uint_typedefs.add(G.word_size)
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
				self.__children[i] = CodepointChunk(self.__children[i])
				for ui in self.__children[i].required_uint_typedefs():
					self.__uint_typedefs.add(ui)
		for child_index in range(0, child_nodes):
			child = self.__children[child_index]
			if child is None:
				data = self.__Data(self.level() + 1)
				data.span_first = child_first + child_index * child_node_max_size
				data.span_last = min(data.span_first + child_node_max_size - 1, child_last)
				self.__children[child_index] = CodepointChunk(data)

	def __str__(self):
		self.__finish()
		s = ''
		if self.root() and len(self.__uint_typedefs) > 0:
			for ui in self.__uint_typedefs:
				s += 'using ui{} = std::uint_least{}_t;\n'.format(ui, ui)
			s += '\n'
		if self.has_expression():
			return s + 'return {};'.format(strip_brackets(self.expression(self.root())))
		else:
			exclusions = []
			assumptions = []
			if self.first() > 0 and (self.root() or self.first() > self.span_first()):
				exclusions.append('cp < ' + self.first_lit())
			else:
				assumptions.append('cp >= ' + self.first_lit())
			if self.root() or self.last() < self.span_last():
				exclusions.append('cp > ' + self.last_lit())
			else:
				assumptions.append('cp <= ' + self.last_lit())
			if exclusions:
				s += 'if ({})\n\treturn false;\n'.format(strip_brackets(compound_or(*exclusions)))
			if assumptions:
				s += 'TOML_ASSUME({});'.format(strip_brackets(compound_and(*assumptions)))
				s += '\n'
			if exclusions or assumptions:
				s += '\n'

			summary = "//# chunk summary: {} codepoints from {} ranges (spanning a search area of {})".format(
				len(self),
				self.range().sparse_value_count() + self.range().contiguous_subrange_count(),
				self.span_size()
			)

			if (self.makes_bitmask_table()):
				table_name = 'bitmask_table_' + str(self.level())
				s += 'constexpr ui{} {}[] = \n{{'.format(G.word_size, table_name)
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
					s += fmt_str.format(mask, 'ull' if G.word_size > 32 else 'u')
				element_selector = '(static_cast<ui{}>(cp) - {}) / {}'.format(
					G.word_size,
					make_bitmask_literal(self.first(), G.word_size),
					make_bitmask_literal(G.word_size, G.word_size)
				)
				bit_selector = 'static_cast<ui{}>(cp)'.format(G.word_size)
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
			emittables_all_have_expressions = True
			defaulted = 0
			for i in range(0, len(self.__children)):
				if ((always_true_selector and self.__children[i].always_returns_true())
					or (always_false_selector and self.__children[i].always_returns_false())):
					continue
				if (default_check and default_check(self.__children[i])):
					defaulted += 1
					continue
				emittables.append((i,self.__children[i]))
				emittables_all_have_expressions = emittables_all_have_expressions and self.__children[i].has_expression()
			if defaulted == 0:
				default = None

			selector = self.child_selector()
			selector_name = 'child_index_{}'.format(self.level())
			if selector_references > 1:
				s += 'const auto {} = {};\n'.format(selector_name, selector)

			requires_switch = len(emittables) > 1 or not emittables_all_have_expressions
			return_trues = []
			if always_true_selector:
				return_trues.append(always_true_selector)
			elif always_false_selector and not expressions_or_switches:
				return_trues.append('!({})'.format(always_false_selector))
				always_false_selector = None
			if not requires_switch:
				return_trues += [e[1].expression() for e in emittables if e[1].has_expression()]
			
			return_falses = []
			if always_false_selector:
				return_falses.append(always_false_selector)

			for l, v in [(return_trues, True), (return_falses, False)]:
				if not l:
					continue
				ret = '\n\t|| '.join(l)
				if (return_trues and return_falses) or requires_switch or default is not None:
					s += 'if ({})\n\treturn {};\n'.format(ret, 'true' if v else 'false')
				else:
					s += 'return {}{}{};'.format(
						'' if v else '!(',
						strip_brackets(ret),
						'' if v else ')'
					)

			if len(emittables) == 0 and default is not None:
				s += 'return {};\n'.format(str(default).lower())
			elif not requires_switch:
				if default is True:
					s += 'return ((@@SELECTOR@@) != {})\n\t|| ({});'.format(
						emittables[0][0],
						strip_brackets(emittables[0][1].expression())
					)
				elif default is False:
					s += 'return ((@@SELECTOR@@) == {})\n\t&& ({});'.format(
						emittables[0][0],
						strip_brackets(emittables[0][1].expression())
					)
				else:
					selector_references -= 1
					s += 'return {};'.format(strip_brackets(emittables[0][1].expression()))
			else:
				s += "switch (@@SELECTOR@@)\n"
				s += "{\n"
				emitted = 0
				for i, c in emittables:
					s += '\tcase 0x{:02X}:{}{}{}'.format(
						i,
						' ' if c.has_expression() else ' // [{}] {:04X} - {:04X}\n\t{{\n'.format(i, c.span_first(), c.span_last()),
						indent_with_tabs(str(c), 0 if c.has_expression() else 2),
						'\n' if c.has_expression() else '\n\t}\n',
					)
					emitted += 1
				s += '\t{};\n'.format('TOML_NO_DEFAULT_CASE' if default is None else 'default: return '+str(default).lower())
				s += "}"
				if (emitted <= 1):
						s += "\n/* FIX ME: switch has only {} case{}! */".format(emitted, 's' if emitted > 1 else '')
				s += '\n' + summary

			if selector_references > 0:
				s = s.replace('@@SELECTOR@@', selector_name if selector_references > 1 else selector)
			return s



##### FUNCTION GENERATORS #############################################################################################



def emit_function(name, header_file, test_file, codepoints, test_func, description):
	root_chunk = CodepointChunk()
	for cp in codepoints:
		if test_func is None or test_func(cp):
			root_chunk.add(cp[0])

	header = lambda txt: print(txt, file=header_file)
	header("	// " + ("\n\t//# ".join(description.split('\n'))))
	header('	[[nodiscard]]')
	header('	TOML_ATTR(const)')
	header('	constexpr bool {}(char32_t cp) noexcept'.format(name))
	header('	{')
	header(indent_with_tabs(str(root_chunk), 2))
	header('	}')
	header('')

	if not test_file:
		return
	test = lambda txt: print(txt, file=test_file)
	test('TEST_CASE("unicode - {}")'.format(name))
	test('{')
	test('	static constexpr auto fn = {};'.format(name))

	if root_chunk.range().contiguous_subrange_count():
		test('')
		test('	// contiguous ranges of values which should return true')
		test('	static constexpr codepoint_range inclusive_ranges[] = ')
		test('	{')
		test('		'+'\n		'.join([' '.join(r) for r in chunks(
			['{{ {}, {} }},'.format(make_literal(f), make_literal(l)) for f, l in root_chunk.range().contiguous_subranges()], 3
		)]))
		test('	};')
		test('	for (const auto& r : inclusive_ranges)')
		test('		REQUIRE(in(fn, r));')

	if root_chunk.range().sparse_value_count():
		test('')
		test('	// individual values which should return true')
		test('	static constexpr char32_t inclusive_values[] = ')
		test('	{')
		test('		'+'\n		'.join([' '.join(r) for r in chunks(
			['{},'.format(make_literal(v)) for v in root_chunk.range().sparse_values()], 6
		)]))
		test('	};')
		test('	for (auto v : inclusive_values)')
		test('		REQUIRE(fn(v));')

	unicode_max = 0x10FFFF
	if len(root_chunk.range()) < (unicode_max + 1):
		exclusive_values = SparseRange()
		low_iter = iter(root_chunk.range())
		high_iter = iter(root_chunk.range())
		try:
			high = next(high_iter)
			while True:
				low = next(low_iter)
				high = next(high_iter)
				if low+1 < high:
					exclusive_values.add(low+1, high-1)
		except StopIteration:
			pass
		if root_chunk.range().first() > 0:
			exclusive_values.add(0, root_chunk.range().first()-1)
		if root_chunk.range().last() < unicode_max:
			exclusive_values.add(root_chunk.range().last()+1, unicode_max)
		exclusive_values.finish()

		if exclusive_values.contiguous_subrange_count():
			test('')
			test('	// contiguous ranges of values which should return false')
			test('	static constexpr codepoint_range exclusive_ranges[] = ')
			test('	{')
			test('		'+'\n		'.join([' '.join(r) for r in chunks(
				['{{ {}, {} }},'.format(make_literal(f), make_literal(l)) for f, l in exclusive_values.contiguous_subranges()], 3
			)]))
			test('	};')
			test('	for (const auto& r : exclusive_ranges)')
			test('		REQUIRE(not_in(fn, r));')

		if exclusive_values.sparse_value_count():
			test('')
			test('	// individual values which should return false')
			test('	static constexpr char32_t exclusive_values[] = ')
			test('	{')
			test('		'+'\n		'.join([' '.join(r) for r in chunks(
				['{},'.format(make_literal(v)) for v in exclusive_values.sparse_values()], 6
			)]))
			test('	};')
			test('	for (auto v : exclusive_values)')
			test('		REQUIRE(!fn(v));')

	test('}')
	test('')



def emit_category_function(name, header_file, test_file, codepoints, categories, exclusions = None):
	emit_function(
		name, header_file, test_file, codepoints,
		lambda cp: (True if exclusions is None else cp[0] not in exclusions) and cp[1] in categories,
		'Returns true if a codepoint belongs to any of these categories:\n\t{}'.format(', '.join(categories))
	)



def emit_character_function(name, header_file, test_file, codepoints, *characters):
	rng = SparseRange()
	for c in characters:
		if isinstance(c, int):
			rng.add(c)
		elif isinstance(c, str):
			rng.add(ord(c))
		elif isinstance(c, tuple) and len(c) == 2:
			rng.add(
				ord(c[0]) if isinstance(c[0], str) else c[0],
				ord(c[1]) if isinstance(c[1], str) else c[1]
			)
		else:
			raise Exception("Invalid argument")
	rng.finish()
	emit_function(
		name, header_file, test_file, codepoints,
		lambda cp: cp[0] in rng,
		'Returns true if a codepoint matches {}:\n\t{}'.format(
			'any of' if len(rng) > 1 else '',
			rng.stringify(lambda v: chr(v) if 32 < v < 127 and chr(v).isprintable() else ('U+{:08X}'.format(v) if v > 0xFFFF else 'U+{:04X}'.format(v)))
		)
	)



#### MAIN #############################################################################################################



class CodeUnit(object):

	__types = {
		'char': 8,
		'char8_t': 8,
		'char16_t': 16,
		'char32_t': 32
	}

	def __init__(self, typename):
		assert isinstance(typename, str)
		assert typename in self.__types

		self.typename = typename
		self.bits = self.__types[typename]
		self.signed = True if typename == 'char' else False # not strictly true but yolo
		self.max = min(0x10FFFF, (1 << (self.bits - (1 if self.signed else 0))) - 1)
		self.equivalent_integer = 'uint8_t' if typename == 'char' else f'std::uint_least{self.bits}_t'
		self.can_represent_ascii = True
		self.can_represent_any_unicode = self.max >= 128
		self.can_represent_all_unicode = self.max >= 0x10FFFF


def write_function(file, code_points, code_units, name, description, categories=None, characters=None):
	assert file is not None
	assert code_points is not None
	assert isinstance(name, str)
	assert isinstance(description, str)
	assert categories is not None or characters is not None
	assert code_units is not None
	if not utils.is_collection(code_units):
		code_units = (code_units,)
	assert len(code_units) > 0

	local_max = 0
	for cu in code_units:
		local_max = max(local_max, cu.max)

	allowed_code_points = SparseRange()
	if categories is not None:
		if not utils.is_collection(categories):
			categories = (categories,)
		for cp, cat in code_points:
			if cp > local_max:
				break
			if cat in categories:
				allowed_code_points.add(cp)

	if characters is not None:
		if not utils.is_collection(characters):
			characters = (characters,)
		for c in characters:
			cp = c
			if isinstance(cp, str):
				cp = ord(cp)
			if isinstance(cp, int):
				if cp <= local_max:
					allowed_code_points.add(cp)
			elif isinstance(cp, tuple) and len(cp) == 2:
				cp = range_intersection(
					ord(cp[0]) if isinstance(c[0], str) else cp[0],
					ord(cp[1]) if isinstance(c[1], str) else cp[1],
					0,
					local_max
				)
				if cp is not None:
					allowed_code_points.add(cp[0], cp[1])
			else:
				raise Exception("Invalid argument")

	allowed_code_points.finish()
	if (not allowed_code_points):
		return False

	chunks = dict()
	write = lambda txt,end='\n': print(txt, file=file, end=end)
	for cu in code_units:
		if allowed_code_points.first() > cu.max:
			continue
		if cu.max not in chunks:
			chunk = CodepointChunk()
			for cp in allowed_code_points:
				if cp > cu.max:
					break
				chunk.add(cp)

		write('\t/// \\brief\t\t' + ("\n\t///\t\t\t\t".join(description.split('\n'))))
		write('\t[[nodiscard]]')
		write('\tTOML_ATTR(const)')
		write(f'\tconstexpr bool {name}({cu.typename} c) noexcept'')
		write('\t{')
		#write(indent_with_tabs(str(chunks[cu.max]), 2))
		write('\t}')
		write('')

	return True

	


def write_header(code_points, code_units, file_path, description):
	assert code_points is not None
	assert isinstance(file_path, str)
	assert isinstance(description, str)
	assert code_units is not None
	if not utils.is_collection(code_units):
		code_units = (code_units,)
	assert len(code_units) > 0

	print("Writing to {}".format(file_path))
	with open(file_path, 'w', encoding='utf-8', newline='\n') as file:
		
		# preamble
		write = lambda txt,end='\n': print(txt, file=file, end=end)
		write('//# This file is a part of muu and is subject to the the terms of the MIT license.')
		write('//# Copyright (c) 2020 Mark Gillard <mark.gillard@outlook.com.au>')
		write('//# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		write('// SPDX-License-Identifier: MIT')
		write('//#-----')
		write('//# this file was generated by generate_unicode_functions.py - do not modify it directly')
		write('')
		write('/// \file')
		write(f'/// \brief {description}')
		write('#pragma once')
		write('#include "../muu/core.h"')
		write('')
		write('MUU_NAMESPACE_START')
		write('{')

		write_function(file, code_points, code_units, 'is_hexadecimal_digit', characters=(('a', 'f'), ('A', 'F'), ('0', '9')))

		# finishing up
		write('}')
		write('MUU_NAMESPACE_END')
		write('')

	
	# header('#ifndef DOXYGEN')
	# header('')
	# header('TOML_IMPL_NAMESPACE_START')
	# header('{')
# 
	# test('#include "tests.h"')
	# test('#include "unicode.h"')
	# test('using namespace toml::impl;')
	# test('')
# 
	# emit_character_function('is_hexadecimal_digit', header_file, test_file, codepoints, ('a', 'f'), ('A', 'F'), ('0', '9'))
# 
	# both('\t#if TOML_LANG_UNRELEASED // toml/issues/687 (unicode bare keys)')
	# both('')
	# unicode_exclusions = SparseRange()
	# unicode_exclusions.add(0, 127) # ascii block
	# unicode_exclusions.finish()
	# emit_category_function('is_unicode_letter', header_file, test_file, codepoints, ('Ll', 'Lm', 'Lo', 'Lt', 'Lu'), unicode_exclusions)
	# emit_category_function('is_unicode_number', header_file, test_file, codepoints, ('Nd', 'Nl'), unicode_exclusions)
	# emit_category_function('is_unicode_combining_mark', header_file, test_file, codepoints, ('Mn', 'Mc'), unicode_exclusions)
	# both('\t#endif // TOML_LANG_UNRELEASED')
# 
	# header('')
	# header('}')
	# header('TOML_IMPL_NAMESPACE_END')
	# header('')
	# header('#endif // !DOXYGEN')



def main():

	# get unicode character database
	codepoint_list = utils.read_all_text_from_file(
		path.join(utils.get_script_folder(), 'UnicodeData.txt'),
		'https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt'
	)

	# parse the database file into codepoints
	re_codepoint = re.compile(r'^([0-9a-fA-F]+);(.+?);([a-zA-Z]+);')
	current_range_start = -1
	codepoints = []
	parsed_codepoints = 0
	for codepoint_entry in codepoint_list.split('\n'):
		match = re_codepoint.search(codepoint_entry)
		if (match is None):
			if (current_range_start > -1):
				raise Exception('Previous codepoint indicated the start of a range but the next one was null')
			continue
		codepoint = int('0x{}'.format(match.group(1)), 16)
		if (current_range_start > -1):
			for cp in range(current_range_start, codepoint):
				parsed_codepoints += 1
				codepoints.append((cp, match.group(3)))
			current_range_start = -1
		else:
			if (match.group(2).endswith(', First>')):
				current_range_start = codepoint
			else:
				parsed_codepoints += 1
				codepoints.append((codepoint, match.group(3)))
	print("Extracted {} of {} codepoints from unicode database file.".format(len(codepoints), parsed_codepoints))
	codepoints.sort(key=lambda r:r[0])

	header_root = path.join(utils.get_script_folder(), '..', 'include', 'muu')

	write_header(code_points, (CodeUnit('char'),)), path.join(header_root, 'utf8.h'), 'utf8'):


if __name__ == '__main__':
	utils.run(main)
