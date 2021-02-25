#!/usr/bin/env python3
# This file is a part of muu and is subject to the the terms of the MIT license.
# Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>
# See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.
# SPDX-License-Identifier: MIT

# conversions between character types of the same size: https://godbolt.org/z/KeddME

import sys
import os.path as path
import utils
import re
import math
import bisect
import json


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
	doxygen						= True
	asserts						= False



def exid(id):
	if G.expression_ids:
		return f' /* exid({id}) */'
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



def __strip_brackets(s):
	if not (s.startswith('(') and s.endswith(')')):
		return (s, False)
	current_depth = 0
	pairs = 0
	for c in s:
		if c == '(':
			if current_depth == 0 and pairs > 0:
				return (s, False)
			current_depth = current_depth + 1
		elif c == ')':
			current_depth = current_depth - 1
			pairs = pairs + 1
	if current_depth == 0:
		return (s[1:-1], True)
	return (s, False)



def strip_brackets(s):
	result = __strip_brackets(s)
	while result[1]:
		result = __strip_brackets(result[0])
	return result[0]



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



def wrap_lines(s, sep = '||', wrap_prefix = '\t', assumed_indent = 0):
	elems = [s.strip() for s in s.split(sep)]
	line_len = 0
	s = ''
	wrap_prefix_len = 0
	for c in wrap_prefix:
		wrap_prefix_len += 4 if c == '\t' else 1
	for e in elems:
		if line_len + len(e) + assumed_indent >= 120:
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
		self.__sparse_value_count = 0
		self.__contiguous_subrange_count = 0
		self.__sparse_values = None
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
			self.__values = []
			self.__ranges = None
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
			if isinstance(elem, (tuple, list)):
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
			if isinstance(v, (tuple, list)):
				yield v

	def sparse_value_count(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		return self.__sparse_value_count

	def sparse_values(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		for v in self.__values:
			if not isinstance(v, (tuple, list)):
				yield v

	def serialize(self):
		if not self.finished():
			raise Exception('finish() has not been called')
		vals = dict()
		vals['_SparseRange__first'] = self.__first
		vals['_SparseRange__last'] = self.__last
		vals['_SparseRange__count'] = self.__count
		vals['_SparseRange__sparse_value_count'] = self.__sparse_value_count
		vals['_SparseRange__contiguous_subrange_count'] = self.__contiguous_subrange_count
		vals['_SparseRange__values'] = self.__values
		return vals

	def deserialize(self, vals):
		self.__dict__.clear()
		self.__dict__.update(vals)
		self.__ranges = None
		self.__sparse_values = None

	def inverse(self, min_, max_):
		inv = SparseRange()
		if len(self) == 0:
			inv.add(min_, max_)
		else:
			low_iter = iter(self)
			high_iter = iter(self)
			try:
				high = next(high_iter)
				while True:
					low = next(low_iter)
					high = next(high_iter)
					if low+1 < high:
						inv.add(low+1, high-1)
			except StopIteration:
				pass
			#print(f'{self.first()} - {self.last()} ({len(self)})')
			if self.first() > min_:
				inv.add(min_, self.first()-1)
			if self.last() < max_:
				inv.add(self.last()+1, max_)
		inv.finish()
		return inv


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
			self.__data.span_first = 0
			self.__data.span_last = min(code_unit.max, 0x10FFFF)

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

	def closed(self):
		return self.__expr_handles_low_end and self.__expr_handles_high_end

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
			if not self.__expr_handles_low_end and self.span_first() > 0:
				bools.append(f'{self.span_first_lit()} <= c' + exid(50))
			if not self.__expr_handles_high_end and self.span_last() < self.__code_unit.max:
				bools.append(f'c <= {self.span_last_lit()}' + exid(51))
			if len(bools) == 0 or not self.always_returns_true():
				bools.append(self.__expr)
			return compound_and(*bools) + exid(16)

	def add(self, first, last = None):
		if self.__finished:
			raise Exception('the chunk is read-only')
		self.range().add(first, last)

	class __Expression:
		def __init__(self, base_score, expr, handles_low=False, handles_high=False):
			self.expr = expr
			self.handles_low = handles_low
			self.handles_high = handles_high
			self.score = int(base_score * (
				1.0													\
				+ (0.05 if handles_low else 0.0)					\
				+ (0.05 if handles_high else 0.0) 					\
				+ (0.15 if handles_low and handles_high else 0.0)))

	def __finish(self):
		if self.__finished:
			return
		if not self.range().finished():
			self.range().finish()
		self.__finished = True
		if self.root():
			if self.__data.span_first is None:
				self.__data.span_first = self.first()
			if self.__data.span_last is None:
				self.__data.span_last = self.last()
		if self.range():
			assert self.first() >= self.span_first()
			assert self.last() <= self.span_last()

		# try to figure out a return expression if possible.

		expressions = []

		# false
		if self.always_returns_false():
			expressions.append(self.__Expression(
				1000,
				f'false{exid(0)}',
				True,
				#True
				not self.root() or self.__code_unit.max < 0x10FFFF
			))

		# true
		if self.always_returns_true():
			expressions.append(self.__Expression(
				1000,
				f'true{exid(1)}',
				True,
				not self.root() or self.__code_unit.max < 0x10FFFF
			))

		# c != A
		if (len(self) == self.span_size() - 1):
			gap = None
			for i in range(self.span_first(), self.span_last()+1):
				if i not in self.range():
					gap = i
					break
			assert gap is not None
			expressions.append(self.__Expression(
				750,
				'c != ' + self.__code_unit.literal(gap) + exid(2),
				gap == self.span_first(),
				gap == self.span_last()
			))

		# c == A
		# A <= c
		# c <= A
		# A <= c && c <= B
		if len(self) > 0 and len(self) < self.span_size() and self.range().contiguous():
			if len(self) == 1:
				expressions.append(self.__Expression(
					750,
					'c == ' + self.first_lit() + exid(3),
					True,
					True
				))
			elif (self.first() > self.span_first()) and (self.last() < self.span_last()):
				expressions.append(self.__Expression(
					750,
					f'({self.first_lit()} <= c && c <= {self.last_lit()})' + exid(4),
					True,
					True
				))
			elif self.last() < self.span_last():
				assert self.first() == self.span_first()
				expressions.append(self.__Expression(
					750,
					f'c <= {self.last_lit()}' + exid(5),
					False,
					True
				))
			else:
				assert self.first() > self.span_first(), f"{self.first()} {self.span_first()}"
				assert self.last() == self.span_last()
				expressions.append(self.__Expression(
					750,
					f'{self.first_lit()} <= c' + exid(6),
					True,
					False
				))

		# c % A == 0
		# (c + A) % B == 0
		if len(self) > 0:
			stop_searching = False
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
						score = 500
						if (add):
							s = f'({s} + {add}u)'
							score = score - 10
						bools = [ f'({s} % {div}u) == 0u' ]
						handles_low = False
						handles_high = False
						if (self.last() < self.span_last()):
							bools.insert(0, f'c <= {self.last_lit()}')
							handles_high = True
						if (self.first() > self.span_first()):
							bools.insert(0, f'{self.first_lit()} <= c')
							handles_low = True
						score = score - 10 * (len(bools) - 1)
						expressions.append(self.__Expression(
							score,
							compound_and(*bools) + exid(7),
							handles_low,
							handles_high
						))
						stop_searching = True
						break
				if stop_searching:
					break

		# c & A
		if len(self) > 0 and G.bitmask_expressions and (self.last() - self.first() + 1) <= G.word_size:
			bitmask = 0
			for i in self.range():
				shift = i - self.first()
				if shift >= G.word_size:
					break
				bitmask |= 1 << shift
			bools = [ make_bitmask_index_test_expression('c', bitmask, -self.first()) ]
			handles_low = False
			handles_high = False
			score = 500
			if (self.last() < self.span_last()):
				bools.insert(0, f'c <= {self.last_lit()}')
				handles_high = True
			if (self.first() > self.span_first()):
				bools.insert(0, f'{self.first_lit()} <= c')
				handles_low = True
			score = score - 10 * len(bools)
			expressions.append(self.__Expression(
				score,
				wrap_lines(compound_and(*bools), sep='&&', wrap_prefix='\t\t', assumed_indent=self.level()*8)  + exid(8),
				handles_low,
				handles_high
			))

		child_first = self.first()
		child_last = self.last()
		child_span = 0 if len(self) == 0 else (child_last - child_first + 1)
		subdivision_allowed = (
			(G.depth_limit <= 0 or (self.level()+1) < G.depth_limit)
			and child_span > 4
			and calc_child_size(child_span) < child_span
		)

		# (c >= A && c <= B) || c == C || c == D ...
		if len(self) > 0 and (	\
			(self.range().sparse_value_count() + self.range().contiguous_subrange_count()) <= G.compound_boolean_limit	\
				or not subdivision_allowed):
			score = 500
			bools = []
			comps = 0
			closed = True
			for f, l in self.range().contiguous_subranges():
				ands = []
				if f > self.span_first():
					ands.append(f'{self.__code_unit.literal(f)} <= c')
				if l < self.span_last():
					ands.append(f'c <= {self.__code_unit.literal(l)}')
				bools.append(f'{compound_and(*ands)}')
				comps = comps + len(ands)
				closed = closed and len(ands) == 2
			for v in self.range().sparse_values():
				bools.append('c == ' + self.__code_unit.literal(v))
				comps = comps + 1
			if self.root() and comps <= 4:
				score += 200
			expressions.append(self.__Expression(
				score - (20 * comps),
				wrap_lines(compound_or(*bools), wrap_prefix='\t\t') + exid(9),
				closed,
				closed
			))


		# if we've identified some candidate expressions, sort them and pick the best one
		if len(expressions) > 0:
			expressions.sort(key=lambda ex: ex.score, reverse=True)
			self.__expr = expressions[0].expr
			self.__expr_handles_low_end = expressions[0].handles_low
			self.__expr_handles_high_end = expressions[0].handles_high
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
		if self.has_expression():
			return 'return {};'.format(strip_brackets(self.expression(self.root())))
		else:
			s = ''
			exclusions = []
			assumptions = []
			if self.span_first() < self.first():
				exclusions.append(f'{self.first_lit()} > c')
			else:
				assumptions.append(f'{self.first_lit()} <= c')
			if self.last() < self.span_last():
				exclusions.append(f'c > {self.last_lit()}')
			else:
				assumptions.append(f'c <= {self.last_lit()}')
			if exclusions:
				s += 'if ({})\n\treturn false{};\n'.format(strip_brackets(compound_or(*exclusions)), exid(10))
			if assumptions:
				s += 'MUU_ASSUME({}){};'.format(strip_brackets(compound_and(*assumptions)), exid(11))
				s += '\n'
			if exclusions or assumptions:
				s += '\n'

			summary = "// {} code units from {} ranges (spanning a search area of {})".format(
				len(self),
				self.range().sparse_value_count() + self.range().contiguous_subrange_count(),
				(self.last() - self.first()) + 1
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
					strip_brackets(element_selector),
					make_bitmask_literal(1, G.word_size),
					strip_brackets(bit_selector)
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
				if len(child) == 1 and child.has_expression() and not child.always_returns_true() and child.closed():
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

			for l, v in [(return_falses, False), (return_trues, True)]:
				if not l:
					continue
				ret = 'true' if v else 'false'
				cond = strip_brackets(wrap_lines(' || '.join(l), assumed_indent=16 + 8 * self.level()).strip())
				if (return_trues and return_falses) or requires_switch or default is not None:
					s += f'if ({cond})\n\treturn {ret}{exid(30)};\n'
				else:
					s += 'return {}{}{}{};'.format(
						'' if v else '!(',
						cond,
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
							indent_with_tabs(strip_brackets(str(c)), 0 if c.has_expression() else 2),
							'\n' if c.has_expression() else '\n\t}\n',
						)
						emitted += 1
					s += '\t{};\n'.format('MUU_NO_DEFAULT_CASE' if default is None else 'default: return '+str(default).lower())
					s += "}"
					if (emitted <= 1):
							s += "\n/* FIX ME: switch has only {} case{}! */".format(emitted, 's' if emitted > 1 else '')
					s += '\n' + summary
					if G.asserts:
						s += '\nMUU_CONSTEXPR_SAFE_ASSERT(false && "unreachable!");'
					else:
						s += '\nMUU_UNREACHABLE;'

			if selector_references > 0:
				s = s.replace('@@SELECTOR@@', selector_name if selector_references > 1 else strip_brackets(selector))
			return s



#### UNICODE DATABASE ##################################################################################################



class Serializer(json.JSONEncoder):
	def default(self, o):
		d = o.serialize()
		d['__serialized_type_key'] = type(o).__name__
		return d



class Deserializer(json.JSONDecoder):
	def __init__(self, *args, **kwargs):
		json.JSONDecoder.__init__(self, object_hook=self.__object_hook, *args, **kwargs)

	def __object_hook(self, dct):
		if '__serialized_type_key' in dct:
			obj_type = globals()[dct['__serialized_type_key']]
			del dct['__serialized_type_key']
			obj = obj_type()
			obj.deserialize(dct)
			return obj
		return dct



class UnicodeDatabase:

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
		json_path = path.join(utils.get_script_folder(), 'Unicode_Properties.json')
		try:
			self.__properties = json.loads(utils.read_all_text_from_file(json_path), cls=Deserializer)
		except:
			self.__properties = dict()
			files = ('PropList.txt', 'DerivedCoreProperties.txt')
			for file in files:
				property_list = utils.read_all_text_from_file(
					path.join(utils.get_script_folder(), f'Unicode_{file}'),
					f'https://www.unicode.org/Public/UCD/latest/ucd/{file}'
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
			print("Writing to {}".format(json_path))
			with open(json_path, 'w', encoding='utf-8', newline='\n') as f:
				f.write(json.dumps(self.__properties, sort_keys=True, indent=4, cls=Serializer))

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
	assert isinstance(h, str)
	global __code_point_range_cache
	code_points = __code_point_range_cache.get(h)
	if code_points is None:
		json_path = path.join(utils.get_script_folder(), f'Unicode_CodePoints_{h}.json')
		if path.exists(json_path):
			try:
				code_points = json.loads(utils.read_all_text_from_file(json_path), cls=Deserializer)
				assert h not in __code_point_range_cache
				__code_point_range_cache[h] = code_points
			except:
				pass
	return code_points


def store_code_point_range(h, code_points):
	assert isinstance(h, str)
	assert isinstance(code_points, SparseRange)
	global __code_point_range_cache
	assert h not in __code_point_range_cache
	__code_point_range_cache[h] = code_points
	json_path = path.join(utils.get_script_folder(), f'Unicode_CodePoints_{h}.json')
	print("Writing to {}".format(json_path))
	with open(json_path, 'w', encoding='utf-8', newline='\n') as f:
		f.write(json.dumps(code_points, sort_keys=True, indent=4, cls=Serializer))



def get_code_points_in_categories(*categories):
	categories = [c.strip() for c in categories]
	categories.sort()
	categories = tuple(categories)
	h = utils.multi_sha256(*categories)
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
	properties = [p.strip() for p in properties]
	properties.sort()
	properties = tuple(properties)
	h = utils.multi_sha256(*properties)
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



class CodeUnit:

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
		self.max = (1 << self.bits) - 1
		self.max_code_point = 127 if self.bits == 8 else min(self.max, 0x10FFFF)
		self.code_point_dead_zone = range(0xD800, 0xDFFF + 1) if self.bits == 16 else None
		self.equivalent_integer = 'uint8_t' if self.bits == 8 else f'uint_least{self.bits}_t'
		self.can_represent_ascii = True
		self.can_represent_any_non_ascii = self.bits > 8
		self.can_represent_all_non_ascii = self.bits > 8 and self.max >= 0x10FFFF
		self.literal_prefix = self.__types[typename][1]
		self.integral_literals = self.typename == 'unsigned char'
		self.proxy = self.typename in ('char', 'wchar_t')
		if self.proxy:
			self.proxy_typename = 'impl::wchar_code_unit' if self.typename == 'wchar_t' else 'unsigned char'

	def is_code_point(self, code_unit):
		return code_unit <= self.max_code_point and (self.code_point_dead_zone is None or code_unit not in self.code_point_dead_zone)

	def literal(self, code_unit):
		if self.integral_literals or not self.is_code_point(code_unit):
			if code_unit > 0xFFFF:
				return f"0x{code_unit:08X}u"
			elif code_unit > 0xFF:
				return f"0x{code_unit:04X}u"
			else:
				return f"0x{code_unit:02X}u"
		else:
			if code_unit < 32:
				esc = self.__escapes.get(code_unit)
				if esc:
					return self.literal_prefix + f"'{esc}'"
			if (32 <= code_unit < 127):
				c = chr(code_unit)
				if c.isprintable():
					return self.literal_prefix + f"'{c}'"
			if code_unit > 0xFFFF:
				return self.literal_prefix + f"'\\U{code_unit:08X}'"
			elif code_unit > 0xFF:
				return self.literal_prefix + f"'\\u{code_unit:04X}'"
			else:
				return self.literal_prefix + f"'\\x{code_unit:02X}'"



#### FUNCTION GENERATORS ###############################################################################################



def write_function_header(files, code_unit, name, return_type, description):
	assert utils.is_collection(files)
	assert len(files) == 2
	assert files[0] is not None
	h = lambda txt,end='\n': print(txt, file=files[0], end=end)
	if G.doxygen:
		h(f'\t/// \\addtogroup\t{name}\t{name}()')
		h('\t/// @{')
		h('')
		h('\t/// \\brief\t\t' + ("\n\t///\t\t\t\t".join(description.split('\n'))))
	h('\t[[nodiscard]]')
	h(f'\tMUU_ATTR{"_NDEBUG" if G.asserts else ""}(const)')
	h(f'\tconstexpr {return_type} {name}({code_unit.typename} c) noexcept')
	h('\t{')



def write_function_body(files, text):
	assert utils.is_collection(files)
	assert len(files) == 2
	assert files[0] is not None
	h = lambda txt,end='\n': print(txt, file=files[0], end=end)
	h(indent_with_tabs(text, 2))



def write_function_footer(files, code_unit):
	assert utils.is_collection(files)
	assert len(files) == 2
	assert files[0] is not None
	h = lambda txt,end='\n': print(txt, file=files[0], end=end)
	h('\t}')
	if G.doxygen:
		h('')
		h('\t/// @}')
	h('')



def add_to_mutex_groups(group_dict, groups, name):
	assert group_dict is not None
	assert groups is not None
	assert name is not None
	if not utils.is_collection(groups):
		groups = (groups,)
	for mx in groups:
		if mx not in group_dict:
			group_dict[mx] = []
		group_dict[mx].append(name)



def write_identification_function(files, code_unit, name, description, categories=None,
		value_ranges=None, properties=None, min_value=0, max_value=0x10FFFF, mutex_groups=None, mutex_groups_dict=None):
	assert utils.is_collection(files)
	assert len(files) == 2
	assert files[0] is not None
	assert isinstance(code_unit, CodeUnit)
	assert isinstance(name, str)
	assert isinstance(description, str)
	assert categories is not None or value_ranges is not None or properties is not None

	# normalize inputs
	min_value = max(min_value, 0)
	max_value = min(max_value, min(code_unit.max_code_point, 0x10FFFF))
	if categories is None:
		categories = False
	elif not utils.is_collection(categories):
		categories = (categories,)
	if value_ranges is None:
		value_ranges = False
	elif not utils.is_collection(value_ranges):
		value_ranges = (value_ranges,)
	if properties is None:
		properties = False
	elif not utils.is_collection(properties):
		properties = (properties,)
	if mutex_groups is not None:
		if not utils.is_collection(mutex_groups):
			mutex_groups = (mutex_groups,)
		add_to_mutex_groups(mutex_groups_dict, mutex_groups, name)


	# get relevant code points and their inverse
	code_points_hash = utils.multi_sha256(code_unit.bits, min_value, max_value, categories, value_ranges, properties)
	code_points = get_code_point_range(code_points_hash)
	inverse_code_points_hash = utils.multi_sha256(code_points_hash, -1)
	inverse_code_points = get_code_point_range(inverse_code_points_hash)
	assert code_points_hash != inverse_code_points_hash
	if code_points is None:
		code_points = SparseRange()
		if categories:
			for cp in get_code_points_in_categories(*categories):
				if cp > max_value:
					break
				if cp < min_value or not code_unit.is_code_point(cp):
					continue
				code_points.add(cp)
		if value_ranges:
			for c in value_ranges:
				cp = c
				if isinstance(cp, str):
					cp = ord(cp)
				if isinstance(cp, int):
					if (min_value <= cp <= max_value) and code_unit.is_code_point(cp):
						code_points.add(cp)
				elif isinstance(cp, tuple) and len(cp) == 2:
					cp = range_intersection(
						ord(cp[0]) if isinstance(c[0], str) else cp[0],
						ord(cp[1]) if isinstance(c[1], str) else cp[1],
						min_value,
						max_value
					)
					if cp is not None:
						for i in range(cp[0], cp[1] + 1):
							if code_unit.is_code_point(i):
								code_points.add(i)
				else:
					raise Exception("Invalid argument")
		if properties:
			for cp in get_code_points_with_properties(*properties):
				if cp > max_value:
					break
				if cp < min_value or not code_unit.is_code_point(cp):
					continue
				code_points.add(cp)
		code_points.finish()
		store_code_point_range(code_points_hash, code_points)
		inverse_code_points = code_points.inverse(0, min(code_unit.max, 0x10FFFF + 10))
		store_code_point_range(inverse_code_points_hash, inverse_code_points)

	# write function
	write_function_header(files, code_unit, name, 'bool', description)
	if not code_points:
		write_function_body(files, f'MUU_UNUSED(c);')
		write_function_body(files, f'return false{exid(69)};')
	elif code_unit.proxy:
		write_function_body(files, f'return {name}(static_cast<{code_unit.proxy_typename}>(c));')
	else:
		chunk = None
		if code_points:
			chunk = CodepointChunk(code_unit)
			for (f,l) in code_points.contiguous_subranges():
				chunk.add(f,l)
			for cp in code_points.sparse_values():
				chunk.add(cp)
		if chunk is not None:
			write_function_body(files, str(chunk))
		else:
			write_function_body(files, 'MUU_UNUSED(c);\nreturn false;')
	write_function_footer(files, code_unit)

	# write tests
	if files[1] != None: 
		t = lambda txt,end='\n': print(txt, file=files[1], end=end)
		t(f'TEST_CASE("unicode - {name} ({code_unit.typename})")')
		t('{')
		t(f'\tstatic constexpr auto fn = static_cast<code_unit_func<{code_unit.typename}>*>({name});')
		iterations = ((code_points, True), (inverse_code_points, False))
		for cps, expected in iterations:
			if cps.contiguous_subrange_count() or cps.sparse_value_count():
				t(' ')
				t(f'\t// values which should return {str(expected).lower()}')
				if cps.contiguous_subrange_count():
					t(f'\tstatic constexpr code_unit_range<uint32_t> {str(expected).lower()}_ranges[] = ')
					t('\t{')
					t('\t\t'+'\n\t\t'.join([' '.join(r) for r in chunks(
						['{{ {}, {} }},'.format(code_unit.literal(f), code_unit.literal(l)) for f, l in cps.contiguous_subranges()], 3
					)]))
					t('\t};')
					t(f'\tfor (const auto& r : {str(expected).lower()}_ranges)')
					if expected and mutex_groups is not None:
						t('\t{')
					t(f'\t\tREQUIRE({"not_" if not expected else ""}in(fn, r));')
					if expected and mutex_groups is not None:
						for mx in mutex_groups:
							t(f'\t\tREQUIRE(in_only<{mx}>(fn, r));')
						t('\t}')
				if cps.sparse_value_count():
					t(f'\tstatic constexpr {code_unit.typename} {str(expected).lower()}_values[] = ')
					t('\t{')
					t('\t\t'+'\n\t\t'.join([' '.join(r) for r in chunks(
						['{},'.format(code_unit.literal(v)) for v in cps.sparse_values()], 6
					)]))
					t('\t};')
					t(f'\tfor (auto v : {str(expected).lower()}_values)')
					if expected and mutex_groups is not None:
						t('\t{')
					t(f'\t\tREQUIRE({"!" if not expected else ""}fn(v));')
					if expected and mutex_groups is not None:
						for mx in mutex_groups:
							t(f'\t\tREQUIRE(in_only<{mx}>(fn, v));')
						t('\t}')
		
		t('}')
		t('')



def write_compound_boolean_function(files, code_unit, name, description, *booleans, mutex_groups=None, mutex_groups_dict=None):
	assert utils.is_collection(files)
	assert len(files) == 2
	assert files[0] is not None
	assert isinstance(code_unit, CodeUnit)
	assert isinstance(name, str)
	assert isinstance(description, str)
	assert booleans is not None

	if mutex_groups is not None:
		if not utils.is_collection(mutex_groups):
			mutex_groups = (mutex_groups,)
		add_to_mutex_groups(mutex_groups_dict, mutex_groups, name)

	write_function_header(files, code_unit, name, 'bool', description)
	if code_unit.proxy:
		write_function_body(files, f'return {name}(static_cast<{code_unit.proxy_typename}>(c));')
	else:
		write_function_body(files, f'return {strip_brackets(compound_or(*booleans))};')
	write_function_footer(files, code_unit)



#### HEADER GENERATOR ##################################################################################################



def write_header(folders, code_unit):
	assert utils.is_collection(folders)
	assert len(folders) == 2
	assert isinstance(folders[0], str)
	assert isinstance(folders[1], str)
	assert isinstance(code_unit, CodeUnit)

	header_path = re.sub(r'\s+', '_', code_unit.typename)
	tests_path = path.join(folders[1], f'unicode_{header_path}.cpp')
	header_path = path.join(folders[0], f'unicode_{header_path}.h')
	print("Writing to {}".format(header_path))
	with open(header_path, 'w', encoding='utf-8', newline='\n') as header_file:
		h = lambda txt,end='\n': print(txt, file=header_file, end=end)

		tests_file = None
		t = None
		both = None
		if code_unit.typename != 'wchar_t':
			print("Writing to {}".format(tests_path))
			tests_file = open(tests_path, 'w', encoding='utf-8', newline='\n')
			t = lambda txt,end='\n': print(txt, file=tests_file, end=end)
			both = lambda txt,end='\n': (h(txt,end), t(txt,end))
		else:
			both = h

		# common preamble
		both('// This file is a part of muu and is subject to the the terms of the MIT license.')
		both('// Copyright (c) Mark Gillard <mark.gillard@outlook.com.au>')
		both('// See https://github.com/marzer/muu/blob/master/LICENSE for the full license text.')
		both('// SPDX-License-Identifier: MIT')
		both('//-----')
		both('// this file was generated by generate_unicode_functions.py - do not modify it directly')
		both('')

		# header preamble
		h('#pragma once')
		if code_unit.typename == 'char':
			h('#include "unicode_unsigned_char.h"')
		elif code_unit.typename == 'wchar_t':
			h('#include "../preprocessor.h"')
			h('#if MUU_WCHAR_BITS == 32')
			h('\t#include "unicode_char32_t.h"')
			h('#elif MUU_WCHAR_BITS == 16')
			h('\t#include "unicode_char16_t.h"')
			h('#elif MUU_WCHAR_BITS == 8')
			h('\t#include "unicode_unsigned char.h"')
			h('#endif')
		else:
			h('#include "../fwd.h"')
		h('')
		h('#include "header_start.h"')
		if not code_unit.proxy:
			h('MUU_PRAGMA_MSVC(warning(disable: 26819)) // core guidelines: Unannotated fallthrough between switch labels')
			h('MUU_PRAGMA_GCC_LT(9, optimize("O1"))')
		h('')
		h('namespace muu')
		h('{')
		if G.doxygen:
			h('\t/// \\addtogroup strings')
			h('\t/// @{')
			h('')
			h('\t/// \\addtogroup code_units')
			h('\t/// @{')
			h('')

		# tests preamble
		if t is not None:
			t('#include "tests.h"')
			if code_unit.typename == 'char8_t':
				t('')
				t('#ifdef __cpp_char8_t')
				t('')
			t('#include "unicode.h"')
			t('#include "../include/muu/strings.h"')
			t('')

		specifier = None
		if code_unit.typename == 'char' or code_unit.typename == 'unsigned char':
			specifier = 'character'
		elif code_unit.typename == 'wchar_t':
			specifier = 'wide character'
		else:
			specifier = f'UTF-{code_unit.bits} code unit'

		files = (header_file, tests_file)
		mutex_groups = dict()


		######### is_ascii_XXXXXXX


		write_identification_function(files, code_unit,
			'is_ascii_code_point',
			f'Returns true if a {specifier} is a valid code point from the ASCII range.',
			value_ranges=((0,127),),
			mutex_groups=1,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_hyphen',
			f'Returns true if a {specifier} is a hyphen code point from the ASCII range.',
			properties='Hyphen',
			max_value=127,
			mutex_groups=2,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_letter',
			f'Returns true if a {specifier} is a letter code point from the ASCII range.',
			categories=('Ll', 'Lm', 'Lo', 'Lt', 'Lu'),
			max_value=127,
			mutex_groups=3,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_lowercase',
			f'Returns true if a {specifier} is a lowercase code point from the ASCII range.',
			properties='Lowercase',
			max_value=127,
			mutex_groups=6,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_number',
			f'Returns true if a {specifier} is a number code point from the ASCII range.',
			categories=('Nd', 'Nl'),
			max_value=127,
			mutex_groups=4,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_uppercase',
			f'Returns true if a {specifier} is an uppercase code point from the ASCII range.',
			properties='Uppercase',
			max_value=127,
			mutex_groups=7,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_ascii_whitespace',
			f'Returns true if a {specifier} is a whitespace code point from the ASCII range.',
			properties='White_Space',
			max_value=127,
			mutex_groups=5,
			mutex_groups_dict=mutex_groups
		)


		######### is_non_ascii_XXXXXXX


		write_identification_function(files, code_unit,
			'is_non_ascii_code_point',
			f'Returns true if a {specifier} is a valid code point from outside the ASCII range.',
			value_ranges=((128,0xFFFFFFFF),),
			mutex_groups=1,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_hyphen',
			f'Returns true if a {specifier} is a hyphen code point from outside the ASCII range.',
			properties='Hyphen',
			min_value=128,
			mutex_groups=2,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_letter',
			f'Returns true if a {specifier} is a letter code point from outside the ASCII range.',
			categories=('Ll', 'Lm', 'Lo', 'Lt', 'Lu'),
			min_value=128,
			mutex_groups=3,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_lowercase',
			f'Returns true if a {specifier} is a lowercase code point from outside the ASCII range.',
			properties='Lowercase',
			min_value=128,
			mutex_groups=6,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_number',
			f'Returns true if a {specifier} is a number code point from outside the ASCII range.',
			categories=('Nd', 'Nl'),
			min_value=128,
			mutex_groups=4,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_uppercase',
			f'Returns true if a {specifier} is an uppercase code point from outside the ASCII range.',
			properties='Uppercase',
			min_value=128,
			mutex_groups=7,
			mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_non_ascii_whitespace',
			f'Returns true if a {specifier} is a whitespace code point from outside the ASCII range.',
			properties='White_Space',
			min_value=128,
			mutex_groups=5,
			mutex_groups_dict=mutex_groups
		)


		######### is_not_XXXXXXX (compound negations)


		write_compound_boolean_function(files, code_unit,
			'is_not_code_point',
			f'Returns true if a {specifier} is not a valid code point.',
			f'{code_unit.literal(0x10FFFF)} < c' if code_unit.max >= 0x10FFFF else '!is_ascii_code_point(c) && !is_non_ascii_code_point(c)',
			mutex_groups=1,
			mutex_groups_dict=mutex_groups,
		)

		write_compound_boolean_function(files, code_unit,
			'is_not_hyphen',
			f'Returns true if a {specifier} is not a hyphen code point.',
			'!is_ascii_hyphen(c) && !is_non_ascii_hyphen(c)',
			mutex_groups=2,
			mutex_groups_dict=mutex_groups
		)
		write_compound_boolean_function(files, code_unit,
			'is_not_letter',
			f'Returns true if a {specifier} is not a letter code point.',
			'!is_ascii_letter(c) && !is_non_ascii_letter(c)',
			mutex_groups=3,
			mutex_groups_dict=mutex_groups
		)
		write_compound_boolean_function(files, code_unit,
			'is_not_lowercase',
			f'Returns true if a {specifier} is not a lowercase code point.',
			'!is_ascii_lowercase(c) && !is_non_ascii_lowercase(c)',
			mutex_groups=6,
			mutex_groups_dict=mutex_groups
		)
		write_compound_boolean_function(files, code_unit,
			'is_not_number',
			f'Returns true if a {specifier} is not a number code point.',
			'!is_ascii_number(c) && !is_non_ascii_number(c)',
			mutex_groups=4,
			mutex_groups_dict=mutex_groups
		)
		write_compound_boolean_function(files, code_unit,
			'is_not_uppercase',
			f'Returns true if a {specifier} is not an uppercase code point.',
			'!is_ascii_uppercase(c) && !is_non_ascii_uppercase(c)',
			mutex_groups=7,
			mutex_groups_dict=mutex_groups
		)
		write_compound_boolean_function(files, code_unit,
			'is_not_whitespace',
			f'Returns true if a {specifier} is not a whitespace code point.',
			'!is_ascii_whitespace(c) && !is_non_ascii_whitespace(c)',
			mutex_groups=5,
			mutex_groups_dict=mutex_groups
		)



		######### compound/special functions



		write_identification_function(files, code_unit,
			'is_code_point',
			f'Returns true if a {specifier} is a valid code point.',
			value_ranges=((0,0xFFFFFFFF),)
		)
		#
		write_function_header(files, code_unit,
			'is_code_point_boundary',
			'bool',
			f"Returns true if a {specifier} is a code point boundary."
		)
		if code_unit.proxy:
			write_function_body(files, f'return is_code_point_boundary(static_cast<{code_unit.proxy_typename}>(c));')
		elif code_unit.bits == 32:
			write_function_body(files, f'return c <= {code_unit.literal(0x0010FFFF)};')
		elif code_unit.bits == 16:
			write_function_body(files, f'return c <= {code_unit.literal(0xDBFF)} || {code_unit.literal(0xE000)} <= c;')
		elif code_unit.bits == 8:
			write_function_body(files, 'return (c & 0b11000000u) != 0b10000000u;')
		write_function_footer(files, code_unit)
		#
		write_identification_function(files, code_unit,
			'is_combining_mark',
			f"Returns true if a {specifier} is a combining mark code point.",
			categories=('Mn', 'Mc')
			#mutex_groups=(7,),
			#mutex_groups_dict=mutex_groups
		)
		write_identification_function(files, code_unit,
			'is_decimal_digit',
			f"Returns true if a {specifier} is a decimal digit code point.",
			value_ranges=(('0', '9'), )
		)
		write_identification_function(files, code_unit,
			'is_hexadecimal_digit',
			f"Returns true if a {specifier} is a hexadecimal digit code point.",
			value_ranges=(('a', 'f'), ('A', 'F'), ('0', '9'))
		)
		write_compound_boolean_function(files, code_unit,
			'is_hyphen',
			f'Returns true if a {specifier} is a hyphen code point.',
			'is_ascii_hyphen(c)', 'is_non_ascii_hyphen(c)'
		)
		write_compound_boolean_function(files, code_unit,
			'is_letter',
			f'Returns true if a {specifier} is a letter code point.',
			'is_ascii_letter(c)', 'is_non_ascii_letter(c)'
		)
		write_compound_boolean_function(files, code_unit,
			'is_lowercase',
			f'Returns true if a {specifier} is a lowercase code point.',
			'is_ascii_lowercase(c)', 'is_non_ascii_lowercase(c)'
		)
		write_compound_boolean_function(files, code_unit,
			'is_number',
			f'Returns true if a {specifier} is a number code point.',
			'is_ascii_number(c)', 'is_non_ascii_number(c)'
		)
		write_identification_function(files, code_unit,
			'is_octal_digit',
			f"Returns true if a {specifier} is an octal digit code point.",
			value_ranges=(('0', '7'), )
		)
		write_compound_boolean_function(files, code_unit,
			'is_uppercase',
			f'Returns true if a {specifier} is an uppercase code point.',
			'is_ascii_uppercase(c)', 'is_non_ascii_uppercase(c)'
		)
		write_compound_boolean_function(files, code_unit,
			'is_whitespace',
			f'Returns true if a {specifier} is a whitespace code point.',
			'is_ascii_whitespace(c)', 'is_non_ascii_whitespace(c)'
		)



		# finish up header
		if G.doxygen:
			h('\t/** @} */	// strings::code_units')
			h('\t/** @} */	// strings')
		h('}')
		h('')
		h('#include "header_end.h"')

		# finish up tests
		if t is not None:
			mutex_groups = [(mx, names) for mx, names in mutex_groups.items()]
			mutex_groups.sort(key=lambda mg: mg[0])
			if mutex_groups:
				for mx, names in mutex_groups:
					t('template <>')
					t(f'struct code_unit_func_group<{code_unit.typename}, {mx}>')
					t('{')
					t(f'\tstatic constexpr code_unit_func<{code_unit.typename}>* functions[] =')
					t('\t{')
					names.sort()
					for n in names:
						t(f'\t\t{n},')
					t('\t};')
					t('};')
					t('')
			if code_unit.typename == 'char8_t':
				t('#endif // __cpp_char8_t')

		if tests_file is not None:
			tests_file.close()

#### MAIN ##############################################################################################################



def main():
	header_folder = path.join(utils.get_script_folder(), '..', 'include', 'muu', 'impl')
	tests_folder = path.join(utils.get_script_folder(), '..', 'tests')
	ucd() # force generation first

	# G.subdivision_allowed = False
	# G.word_size = 32
	# G.compound_boolean_limit = 4
	# G.expression_ids = True
	write_header((header_folder, tests_folder), CodeUnit('char'))
	write_header((header_folder, tests_folder), CodeUnit('unsigned char'))
	write_header((header_folder, tests_folder), CodeUnit('char8_t'))
	write_header((header_folder, tests_folder), CodeUnit('char16_t'))
	write_header((header_folder, tests_folder), CodeUnit('char32_t'))
	write_header((header_folder, tests_folder), CodeUnit('wchar_t'))


if __name__ == '__main__':
	utils.run(main)
