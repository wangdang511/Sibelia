#!/usr/bin/python

from argparse import ArgumentParser
from Bio import SeqIO
from Bio.Seq import Seq
from Bio.Alphabet import IUPAC
from Bio.Blast import NCBIWWW
from Bio.Blast import NCBIXML
import os.path
import re
import sys

ME_THRESHOLD = 2
MAX_INDENT_THRESHOLD = 85

class Block:
	def __init__(self, id, strand, start = 0, end = 0):
		self.id = id
		self.strand = strand
		self.start = start
		self.end = end
	
	def __str__(self):
		return ("+" if self.strand else "-") + str(self.id) + ", start=" + str(self.start) + ", end=" + str(self.end)
	
	def __eq__(self, other):
		return self.id == other.id and self.strand == other.strand
	
	def __ne__(self, other):
		return not self == other

	def reverse(self):
		return Block(self.id, not self.strand, self.start, self.end)

class Permutation:
	def __init__(self, name, id, permutation = []):
		self.name = name
		self.id = id
		self.permutation = permutation
	
	def __str__(self):
		res = "Name: " + self.name + " " + str(self.id) + "\n" + "Permutation:"
		for block in self.permutation:
			res += str(block) + " "
		return res
	
	def reverse(self):
		return [block.reverse() for block in self.permutation[::-1]]
	
	@staticmethod
	def reverse_seq(seq):
		return [block.reverse() for block in seq[::-1]]

	@staticmethod
	def compare_permutations(seq1, seq2):
		if seq1 == seq2:
			return 0
		elif seq1 == Permutation.reverse_seq(seq2):
			return 1
		else:
			return -1

class MobileElement:
	def __init__(self, permutation_id, location, blocks_seq=None, description="", permutation_name=""):
		self.permutation_id = permutation_id
		self.blocks_seq = blocks_seq
		self.location = location
		self.description = description
		self.permutation_name = permutation_name

	def __str__(self):
		res = "Description:" + self.description + "\n"
		res += "Permutation:" + str(self.permutation_id) + " - " + self.permutation_name + "\n"
		res += "Location:" + str(self.location) + "\n"
		res += "Blocks:" + ("\n start id = " + str(self.blocks_seq[0]) + ", end id = " + str(self.blocks_seq[1]) if self.blocks_seq else "None")
#		for block in self.blocks_seq:
#			res += str(block) + " "
		return res

def find_neighbors(permutations, before_me, after_me, before, after):
	for i, block in enumerate(permutations[0].permutation):
		if before_me and before_me.id == block.id:
			before.append(i)
		if after_me and after_me.id == block.id:
			after.append(i)

def find_difference(mobile_elements, permutations, fout):
	for me in mobile_elements:
		if not me.blocks_seq or not me.permutation_id:
			continue
		first_block = permutations[me.permutation_id].permutation[me.blocks_seq[0]]
		last_block = permutations[me.permutation_id].permutation[me.blocks_seq[1]]
		before_me = (permutations[me.permutation_id].permutation[me.blocks_seq[0] - 1]
			if me.blocks_seq[0] > 0 else None)
		after_me = (permutations[me.permutation_id].permutation[me.blocks_seq[1] + 1]
			if me.blocks_seq[1] < len(permutations[me.permutation_id].permutation) - 1 else None)
#		print str(first_block) + " " + str(last_block) + " " + str(before_me) + " " + str(after_me)

		before = []
		after = []
		find_neighbors(permutations, before_me, after_me, before, after)

		for block_id in before:
			if (permutations[0].permutation[block_id + 1] != first_block
					and permutations[0].permutation[block_id - 1] != first_block.reverse()):
#				print str(permutations[0].permutation[block_id + 1]) + " " + str(permutations[0].permutation[block_id - 1]) + " " + str(first_block)
				fout.write("Mobile element: " + str(me) + "\n")
				fout.write("Insert near: " + str(block_id) + " - " + str(permutations[0].permutation[block_id]) + "\n\n")

		for block_id in after:
			if (permutations[0].permutation[block_id - 1] != last_block
					and permutations[0].permutation[block_id + 1] != last_block.reverse()):
				fout.write("Mobile element: " + str(me) + "\n")
				fout.write("Insert near: " + str(block_id) + " - " + str(permutations[0].permutation[block_id]) + "\n\n")

def blast_sequence(sequence, mobile_elements, permutations):
	blast_handle = NCBIWWW.qblast("blastn", "nr", sequence, megablast = True, entrez_query = "insert*[Title] OR transpos*[Title]")
#	out = open("tmp_res.xml", "w")
#	out.write(blast_handle.read())
#	out.close()
#	blast_records = NCBIXML.parse(open("tmp_res.xml"))
	blast_records = NCBIXML.parse(blast_handle)
	record = None
	try:
		for record in blast_records:
			permutation = next((p for p in permutations if p.name == record.query), None)
			if not permutation:
				continue
			for aln in record.alignments:
				for hsp in aln.hsps:
					insert_blast_res_to_mobile_elements(mobile_elements, permutation, hsp.query_start, hsp.query_start + len(hsp.query), aln.title)
	except: 
		print "Unknown error occured " + str(sys.exc_info()) + " " + (" on " + record.query if record else "")
		return

def insert_blast_res_to_mobile_elements(mobile_elements, permutation, query_start, query_end, description):
	start_block = next((i for i, block in enumerate(permutation.permutation) if block.start >= query_start), None)
	end_block = next((len(permutation.permutation) - 1 - i for i, block in enumerate(permutation.permutation[::-1]) 
		if block.end <= query_end), None)
	if start_block is not None and end_block is not None and start_block <= end_block:
		mobile_elements.append(MobileElement(permutation_id = permutation.id, location = (query_start, query_end), blocks_seq = (start_block, end_block), description = description, permutation_name = permutation.name))
	else:
		mobile_elements.append(MobileElement(permutation_id = permutation.id, location = (query_start, query_end), description = description, permutation_name = permutation.name))

def add_block_start_end(permutations, block_id, seq_id, start, end, prev = 0):
	permutation = next(p for p in permutations if p.name == seq_id)
	(move, block) = next(((i, b) for i, b in enumerate(permutation.permutation[prev:]) if b.id == block_id), (0, None))
	if not block:
		print "It should never happen"
		sys.exit(-1)
	block.start = start if start <= end else end
	block.end = end if start <= end else start
	prev += move + 1
	return prev

def parse_permutation_string(perm_str):
	res = []
	for block in perm_str.split(' ')[:-1]:
		res.append(Block(int(block[1:]), block[0] == '+'))
	return res

def parse_permutations_file(filename, permutations):
	perm_file = open(filename)
	for l in perm_file:
		if l[0] == ">":
			permutations.append(Permutation(l.strip()[1:], len(permutations)))
		else:
			permutations[-1].permutation = parse_permutation_string(l.strip())

def parse_sequences_file(filename, permutations):
	prev = {}
	for block_record in SeqIO.parse(filename, "fasta"):
		block_params = {param.split('=')[0]: param.split('=')[1] for param in block_record.id.split(',')}
		block_id, strand, seq_id, start, end = (int(block_params['Block_id']), (block_params['Strand'] == '\'+\''), block_params['Seq'][1:-1], int(block_params['Start']), int(block_params['End']))

		if seq_id not in prev.keys():
			prev[seq_id] = {}
		if block_id not in prev[seq_id]:
			prev[seq_id][block_id] = 0

		prev[seq_id][block_id] = add_block_start_end(permutations, block_id, seq_id, start, end, prev[seq_id][block_id])

def parse_assembly_file(filename, sequences):
	for record in SeqIO.parse(filename, "fasta"):
		sequences.append(record)

if (__name__ == "__main__"):

	script_dir = os.path.dirname(sys.argv[0])

	parser = ArgumentParser(description='Script for mobile elements annotation')
	parser.add_argument("-p", action="store", dest="permutations", help="genome permutations file")
	parser.add_argument("-s", action="store", dest="sequences", help="block sequences file")
	parser.add_argument("-a", action="store", dest="assembly", help="FASTA file with assembly")
	parser.add_argument("-r", action="store", dest="reference", help="FASTA file with reference")
	parser.add_argument("-o", action="store", dest="output", help="output file")

	args = parser.parse_args()

	# define default names if needed
	if (not args.permutations):
		args.permutations = "./genomes_permutations.txt"

	if (not os.path.exists(args.permutations)):
		print "Please, specify permutations file"
		sys.exit(-1)

	if (not args.sequences):
		args.sequences = "./blocks_sequences.fasta"

	if (not os.path.exists(args.sequences)):
		print "Please, specify sequences file"
		sys.exit(-1)

	if (not args.output):
		args.output = "./mobile_elements.txt"
	
	permutations = []
	mobile_elements = []

	parse_permutations_file(args.permutations, permutations)
	parse_sequences_file(args.sequences, permutations)

	if args.assembly and os.path.exists(args.assembly):
		blast_sequence(open(args.assembly).read(), mobile_elements, permutations)

	if args.reference and os.path.exists(args.reference):
		blast_sequence(open(args.reference).read(), mobile_elements, permutations)

	fout = open(args.output, "w")

	for me in mobile_elements:
		fout.write(str(me) + "\n\n")
	
	fout.write("\n\nMobile elements insertions:\n\n")
	find_difference(mobile_elements, permutations, fout)