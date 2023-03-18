import xml.etree.ElementTree as et
import sys

def expect_char(s, i, c):
	if i >= len(s) or c != s[i]:
		print("error: expected character " + c + " at index " + str(i))
		exit()
	return(i+1)

def accept_char(s, i, c):
	if i < len(s) and c == s[i]:
		i += 1
	return(i)

def consume_number(s, i):
	sign = 1
	if s[i] == '+':
		i += 1
	if s[i] == '-':
		sign = -1
		i += 1
	res = 0
	decimalPos = 0.1
	while i < len(s) and s[i].isdigit() :
		res *= 10
		res += int(s[i])
		i += 1
	if i < len(s) and s[i] == '.':
		i += 1
		while i < len(s) and s[i].isdigit() :
			res += decimalPos * int(s[i])
			decimalPos *= 0.1
			i += 1
	return(res*sign, i)

def consume_point(s, i):
	(x, i) = consume_number(s, i)
	i = accept_char(s, i, ',')
	(y, i) = consume_number(s, i)
	return(x, y, i)

def f2s(x):
	return f'{x:.3f}'

class svgContext:
	sp = (0, 0)
	cp = (0, 0)
	rp = (0, 0)

	def reset(self):
		self.cp = self.sp = self.rp = (0,0)

	def move_to(self, rel, x, y):
		if rel:
			x += self.cp[0]
			y += self.cp[1]
		print("\tmg_move_to(" + f2s(x) + ", " + f2s(y) + ");")
		self.sp = (x, y)
		self.cp = (x, y)
		self.rp = self.cp

	def curve_to(self, rel, x1, y1, x2, y2, x3, y3):
		if rel:
			x1 += self.cp[0]
			y1 += self.cp[1]
			x2 += self.cp[0]
			y2 += self.cp[1]
			x3 += self.cp[0]
			y3 += self.cp[1]
		print("\tmg_cubic_to(" + f2s(x1) + ", " + f2s(y1) + ", " + f2s(x2) + ", " + f2s(y2) + ", " + f2s(x3) + ", " + f2s(y3) + ");")
		self.rp = (x2, y2)
		self.cp = (x3, y3)

	def smooth_curve_to(self, rel, x2, y2, x3, y3):
		if rel:
			x2 += self.cp[0]
			y2 += self.cp[1]
			x3 += self.cp[0]
			y3 += self.cp[1]
		x1 = 2*self.cp[0] - self.rp[0]
		y1 = 2*self.cp[1] - self.rp[1]
		print("\tmg_cubic_to(" + f2s(x1) + ", " + f2s(y1) + ", " + f2s(x2) + ", " + f2s(y2) + ", " + f2s(x3) + ", " + f2s(y3) + ");")
		self.rp = (x2, y2)
		self.cp = (x3, y3)

	def line_to(self, rel, x1, y1):
		if rel:
			x1 += self.cp[0]
			y1 += self.cp[1]
		print("\tmg_line_to(" + f2s(x1) + ", " + f2s(y1) + ");")
		self.cp = (x1, y1)
		self.rp = self.cp

	def vertical_to(self, rel, y1):
		if rel:
			y1 += self.cp[1]
		x1 = self.cp[0]
		print("\tmg_line_to(" + f2s(x1) + ", " + f2s(y1) + ");")
		self.cp = (x1, y1)
		self.rp = self.cp

	def close_path(self):
		print("\tmg_close_path();");
		self.cp = self.rp = self.sp

def print_path(path, ctx):
	# print("path " + path.get('id') + ":")
	d = path.get('d')
	index = 0
	c = d[index]

	while index < len(d):
		c = d[index]
		index += 1

		rel = c.islower()
		c = c.lower()

		if c == 'm':
			(x, y, index) = consume_point(d, index)
			ctx.move_to(rel, x, y)

			while index < len(d) and (d[index] == ',' or d[index] == '+' or d[index] == '-'):
				index = accept_char(d, index, ',')
				(x, y, index) = consume_point(d, index)
				ctx.move_to(rel, x, y)

		elif c == 'l':
			(x1, y1, index) = consume_point(d, index)
			ctx.line_to(rel, x1, y1)

			while index < len(d) and (d[index] == ',' or d[index] == '+' or d[index] == '-'):
				index = accept_char(d, index, ',')
				(x1, y1, index) = consume_point(d, index)
				ctx.line_to(rel, x1, y1)

		elif c == 'v':
			(y1, index) = consume_number(d, index)
			ctx.vertical_to(rel, y1)

		elif c == 'c':
			(x1, y1, index) = consume_point(d, index)
			index = accept_char(d, index, ',')
			(x2, y2, index) = consume_point(d, index)
			index = accept_char(d, index, ',')
			(x3, y3, index) = consume_point(d, index)
			ctx.curve_to(rel, x1, y1, x2, y2, x3, y3)

			while index < len(d) and (d[index] == ',' or d[index] == '+' or d[index] == '-'):
				index = accept_char(d, index, ',')
				(x1, y1, index) = consume_point(d, index)
				index = accept_char(d, index, ',')
				(x2, y2, index) = consume_point(d, index)
				index = accept_char(d, index, ',')
				(x3, y3, index) = consume_point(d, index)
				ctx.curve_to(rel, x1, y1, x2, y2, x3, y3)

		elif c == 's':
			(x2, y2, index) = consume_point(d, index)
			index = accept_char(d, index, ',')
			(x3, y3, index) = consume_point(d, index)
			ctx.smooth_curve_to(rel, x2, y2, x3, y3)

			while index < len(d) and (d[index] == ',' or d[index] == '+' or d[index] == '-'):
				index = accept_char(d, index, ',')
				(x2, y2, index) = consume_point(d, index)
				index = accept_char(d, index, ',')
				(x3, y3, index) = consume_point(d, index)
				ctx.smooth_curve_to(rel, x2, y2, x3, y3)


		elif c == 'z':
			ctx.close_path()
			index += 1
		else:
			print('error: unrecognized command')
			exit()


def parse_color(s):
	s = s.lstrip('#')
	if len(s) == 3:
		return (int(s[0]+s[0], 16)/255., int(s[1]+s[1], 16)/255., int(s[2]+s[2], 16)/255.)
	elif len(s) == 6:
		return (int(s[0:2], 16)/255., int(s[2:4], 16)/255., int(s[4:6], 16)/255.)
	else:
		print("error: unrecognized color: " + s)
		exit()

tree = et.parse('./Ghostscript_Tiger.svg')
ctx = svgContext()

print("void draw_tiger()")
print("{")

for g in tree.iter('{http://www.w3.org/2000/svg}g'):

	for path in g.findall('{http://www.w3.org/2000/svg}path'):
		ctx.reset()
		print_path(path, ctx)

		fill = g.get('fill')
		stroke = g.get('stroke')
		stroke_width = g.get('stroke-width')

		if fill != None and fill != "none":
			(r, g, b) = parse_color(fill)
			print("\tmg_set_color_rgba(" + f2s(r) + ", " + f2s(g) + ", " + f2s(b) + ", 1);")
			print("\tmg_fill();")

		if stroke_width != None:
			print("\tmg_set_width(" + stroke_width + ");");

		if stroke != None and stroke != "none":
			(r, g, b) = parse_color(stroke)
			if fill != None:
				ctx.reset()
				print_path(path, ctx)
			print("\tmg_set_color_rgba(" + f2s(r) + ", " + f2s(g) + ", " + f2s(b) + ", 1);")
			print("\tmg_stroke();")

		if (stroke == None or stroke == 'none') and (fill == None or fill == ''):
			print("error, group " + g.get("id") + " has no command")

	print("")

print("}")
