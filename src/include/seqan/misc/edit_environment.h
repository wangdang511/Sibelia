// ==========================================================================
//                 SeqAn - The Library for Sequence Analysis
// ==========================================================================
// Copyright (c) 2006-2010, Knut Reinert, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================

#ifndef SEQAN_HEADER_MISC_EDIT_ENVIRONMENT_H
#define SEQAN_HEADER_MISC_EDIT_ENVIRONMENT_H

namespace SEQAN_NAMESPACE_MAIN
{

    template < typename TDistanceSpec, unsigned DISTANCE /*= 1*/ >
    struct EditEnvironment;

//////////////////////////////////////////////////////////////////////////////
/*DUBLICATED DOCUMENTATION, SEE pipe_edit_environment.h
.Spec.EditEnvironment:
..cat:Pipelining
..general:Class.Pipe
..summary:Outputs tuples of the $tupleLen$ consecutive elements of the input stream.
..signature:Pipe<TInput, Tupler<tupleLen, omitLast> >
..param.TInput:The type of the pipeline module this module reads from.
..param.tupleLen:The tuple length.
...remarks:The tuples contain elements $in[i]in[i+1]...in[i+(tupleLen-1)]$.
..param.omitLast:Omit half filled tuples.
..param.omitLast:If $true$, the output stream is $tupleLen-1$ elements shorter than the input stream.
..param.omitLast:If $false$, the lengths are identical and the last tuples are filled with blanks (default constructed elements) for undefined entries.
..remarks:The output type is a @Class.Tuple@ of input elements and length $tupleLen$ (i.e. $Tuple<Value<TInput>::Type, tupleLen>$).
..remarks:The tuples are sequences of the form $in[i]in[i-1]in[i-2]..in[i-tupleLen+1]$. For $omitLast=false$ $i$ begins with 0 and for $omitLast=true$ $i$ begins with $tupleLen-1$.
..include:seqan/misc.h
*/

    template < typename TObject, typename TSpec >
    class Enumerator
    {
	public:
		Holder<TObject> data_host;
		unsigned		minDist;
		bool			trim;

        Enumerator(TObject& _original):
            data_host(_original),
			minDist(0),
			trim(true) {}

		Enumerator(TObject& _original, unsigned _minDist):
            data_host(_original),
			minDist(_minDist),
			trim(true) {}
	};
		
	template <typename TObject, typename TSpec>
	struct Value< Enumerator<TObject, TSpec> >:
		Value<TObject> {};

	template <typename TObject, typename TSpec>
	struct Reference< Enumerator<TObject, TSpec> > {
		typedef TObject & Type;
	};

	template <typename TObject, typename TSpec>
	struct Reference< Enumerator<TObject, TSpec> const > {
		typedef TObject const & Type;
	};

	template <typename TObject, typename TSpec>
	struct Size< Enumerator<TObject, TSpec> >:
		Size<TObject> {};

	template <typename TObject, typename TSpec>
	struct Difference< Enumerator<TObject, TSpec> >:
		Difference<TObject> {};

	template <typename TObject, typename TSpec>
	struct Position< Enumerator<TObject, TSpec> >:
		Position<TObject> {};

	template <typename TObject, typename TSpec>
	struct Iterator< Enumerator<TObject, TSpec>, Standard > {
		typedef Iter< Enumerator<TObject, TSpec>, Standard > Type;
	};
	template <typename TObject, typename TSpec>
	struct Iterator< Enumerator<TObject, TSpec> const, Standard > {
		typedef Iter< Enumerator<TObject, TSpec>, Standard > Type;
	};

	template <typename TObject, typename TSpec>
	struct Host< Enumerator< TObject, TSpec> > {
		typedef TObject Type;
	};
	template <typename TObject, typename TSpec>
	struct Host< Enumerator< TObject, TSpec> const > {
		typedef TObject const Type;
	};

	template <typename TText, typename TSpec>
	inline Holder<TText> & 
	_dataHost(Enumerator<TText, TSpec> &enumerator) {
		return enumerator.data_host;
	}
	template <typename TText, typename TSpec>
	inline Holder<TText> const & 
	_dataHost(Enumerator<TText, TSpec> const &enumerator) {
		return enumerator.data_host;
	}


//////////////////////////////////////////////////////////////////////////////

	template <typename TSize>
	struct EnumeratorHammingModifier_
	{
		TSize		errorPos;		// position of substitution
		unsigned	character;		// replacement character 
		unsigned	skipChar;		// skip the original character
	};

	template < typename TObject, unsigned DISTANCE >
    class Iter< Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard >
    {
	public:
		typedef typename Value<TObject>::Type			TValue;
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize> TModifier;
        
		TObject									&orig;
//		typename RemoveConst_<TObject>::Type	tmp;
		String<TValue>							tmp;

		TModifier	mod[DISTANCE];
		unsigned	minDist;
		bool		trim;
		
		Iter(TObject& _original):
            orig(_original),
			minDist(0)
		{
			goBegin(*this);
		}

		Iter(TObject& _original, unsigned _minDist, bool _trim):
            orig(_original),
			minDist(_minDist),
			trim(_trim)
		{
			goBegin(*this);
		}

		Iter(TObject& _original, MinimalCtor):
			orig(_original),
			minDist(0) {}

		Iter(TObject& _original, unsigned _minDist, bool _trim, MinimalCtor):
			orig(_original),
			minDist(_minDist),
			trim(_trim) {}
	};

//////////////////////////////////////////////////////////////////////////////

	template < typename TObject, typename TSpec >
    inline TObject const &
	operator * (Iter<Enumerator<TObject, TSpec>, Standard> & it)
	{
		return it.tmp;
	}

	template < typename TObject, typename TSpec >
    inline TObject const &
	operator * (Iter<Enumerator<TObject, TSpec>, Standard> const & it)
	{
		return it.tmp;
	}

//////////////////////////////////////////////////////////////////////////////
// enumerate the hamming 1-environment

	template < typename TObject, unsigned DISTANCE >
    inline void
	goBegin(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> & it)
	{
		typedef typename Value<TObject>::Type			TValue;
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		if (empty(it.orig) || it.minDist > DISTANCE || it.minDist > length(it.orig)) {
			goEnd(it);
			return;
		}

		it.tmp = it.orig;

		unsigned i = 0;
		unsigned mDist = it.minDist;

		if (mDist > length(it.orig))
			mDist = length(it.orig);

		if (mDist == 0) {
			it.mod[0].errorPos = 0;
			it.mod[0].skipChar = -1;
			it.mod[0].character = 0;
			assignValueAt(it.tmp, 0, (TValue) 0);
			i = 1;
		} else
			for(; i < mDist; ++i) {
				TModifier &mod = it.mod[i];
				mod.errorPos = (mDist - 1) - i;
				mod.skipChar = (unsigned) it.orig[mod.errorPos];
				mod.character = (0 == mod.skipChar)? 1: 0;
				assignValueAt(it.tmp, mod.errorPos, (TValue) mod.character);
			}
		for(; i < DISTANCE; ++i) {
			TModifier &mod = it.mod[i];
			mod.errorPos = -1;
			mod.character = 0;
			mod.skipChar = -1;
		}
	}
	
	template < typename TObject, unsigned DISTANCE >
    inline void
	atBegin(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & it)
	{
		Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> tmp(it.orig, it.minDist, it.trim);
		return tmp == it;
	}

	template < typename TObject, unsigned DISTANCE >
    inline bool
	atBegin(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> & it)
	{
		return atBegin(const_cast<Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const &>(it));
	}

	template < typename TObject, unsigned DISTANCE >
    inline void
	goEnd(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> & it)
	{
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier &mod = it.mod[i];
			mod.errorPos = -1;
			mod.character = 0;
		}
	}

	template < typename TObject, unsigned DISTANCE >
    inline bool
	atEnd(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & it)
	{
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier const &mod = it.mod[i];
			if (mod.errorPos != (TSignedSize)-1 || mod.character != 0u)
				return false;
		}
		return true;
	}

	template < typename TObject, unsigned DISTANCE >
    inline bool
	atEnd(Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> & it)
	{
		return atEnd(const_cast<Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const &>(it));
	}

	template < typename TObject, unsigned DISTANCE >
    inline Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> &
	operator ++ (Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> & it) 
	{
		typedef typename Value<TObject>::Type			TValue;
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		for(unsigned i = 0; true;)
		{
			TModifier *mod = &it.mod[i];

			// next replacement value
			if (++mod->character < ValueSize<TValue>::VALUE) {
				// output the original tuple only once
				if (mod->character == mod->skipChar) continue;
				assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);
				break;
			}
			mod->character = (0 == mod->skipChar)? 1: 0;
			assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);

			if (++i == DISTANCE || (mod + 1)->errorPos == (TSignedSize)-1)
			{
				for(i = 0; i < DISTANCE; ++i) 
				{
					mod = &it.mod[i];

					// restore char at old position
					if (mod->errorPos >= 0)
					{
						std::cout<<"org"<<it.orig<<"  tmp"<<it.tmp<<" ___  ";
						assignValueAt(it.tmp, mod->errorPos, it.orig[mod->errorPos]);
						std::cout<<"org"<<it.orig<<"  tmp"<<it.tmp<<std::endl;
					}

					// next error position
					if (++(mod->errorPos) < (TSignedSize)(length(it.tmp) - i)) 
					{
						mod->skipChar = (unsigned) it.orig[mod->errorPos];
						mod->character = (0 == mod->skipChar)? 1: 0;
						assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);

						for(; i > 0;) {
							mod = &it.mod[i];
							it.mod[i - 1].errorPos = mod->errorPos + 1;
							--i;
							mod->skipChar = (unsigned) it.orig[mod->errorPos];
							mod->character = (0 == mod->skipChar)? 1: 0;
							assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);
						}
						return it;
					}
				}
				// end
				for(i = 0; i < DISTANCE; ++i) {
					mod = &it.mod[i];
					mod->errorPos = -1;
					mod->character = 0;
				}
				return it;
			}
		}
        return it;
    }

	template < typename TObject, unsigned DISTANCE >
	inline typename Size< Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> > >::Type
    length(Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> > const & me)
	{
		typedef typename Value<TObject>::Type TValue;
		typedef typename Size<TObject>::Type TSize;

		static const unsigned alphabetSize = ValueSize<TValue>::VALUE;
		TSize sum = 0;
		TSize numerator = 1;
		TSize alpha = 1;
		TSize divisor = 1;

		for(unsigned i = 0; i <= DISTANCE; ++i) 
		{
			if (i >= me.minDist)
				sum += alpha * (numerator / divisor);

			divisor		*= i + 1;
			numerator	*= length(host(me)) - i;
			alpha		*= alphabetSize - 1;
		}

		return sum;
    }


//////////////////////////////////////////////////////////////////////////////
// enumerate the levenshtein 1-environment

	template <typename TSize>
	struct EnumeratorLevenshteinModifier_ 
	{
		enum TState { DISABLED_, SUBST_, DELETE_, INSERT_, Eof_ };
		TSize		errorPosOrig;	// position of edit operation in original string
		TSize		errorPos;		// position of edit operation in modified string
		TSize		errorPosEnd;	// errorPos < errorPosEnd must be fulfilled
		unsigned	character;		// replacement character 
		unsigned	skipChar;		// skip the original character
		TState		state;			// current state subst/insert before/delete
	};

	template < typename TObject, unsigned DISTANCE >
	class Iter< Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard >
	{
	public:
		typedef typename Value<TObject>::Type				TValue;
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize> TModifier;
        
		TObject									&orig;
//		typename RemoveConst_<TObject>::Type	tmp;
		String<TValue>							tmp;

		TModifier	mod[DISTANCE + 1];
		unsigned	minDist;
		unsigned	currentDistance;		// upper bound for dist(original, *this)
		bool		trim;

		Iter(TObject& _original):
            orig(_original),
			minDist(0),
			trim(true)
		{
			goBegin(*this);
		}

		Iter(TObject& _original, unsigned _minDist, bool _trim):
            orig(_original),
			minDist(_minDist),
			trim(_trim)
		{
			goBegin(*this);
		}

		Iter(TObject& _original, MinimalCtor):
			orig(_original),
			minDist(0) {}

		Iter(TObject& _original, unsigned _minDist, bool _trim, MinimalCtor):
			orig(_original),
			minDist(_minDist),
			trim(_trim) {}

		inline bool _reinit(int pos, int posOrig)
		{
			tmp = orig;
			int posOrigEnd = length(tmp);
			typename TModifier::TState lastState = TModifier::DISABLED_;
			// i from high to low  =  modifier from left to right
			for(int i = currentDistance - 1; i >= 0; --i)
			{
				TModifier &_mod = mod[i];
				switch (_mod.state) {
					case TModifier::SUBST_:
						// INSERT+SUBST is SUBST+INSERT (already enumerated)
						// DELETE+SUBST is SUBST+DELETE (already enumerated)
						// eventually trim front SUBSTs
						if (lastState == TModifier::INSERT_ || lastState == TModifier::DELETE_ ||
							(trim && posOrig == 0)) 
						{
							++posOrig;
							++pos;
						}
						if (posOrig >= posOrigEnd)
							return false;
						_mod.errorPosOrig = posOrig;
						_mod.errorPos = pos;
						_mod.skipChar = (unsigned) orig[posOrig];
						_mod.character = (0 == _mod.skipChar)? 1: 0;
						assignValueAt(tmp, pos, (TValue) _mod.character);
						++pos;
						++posOrig;
						break;

					case TModifier::DELETE_:
						// INSERT after DELETE is one SUBST (already enumerated)
						if (lastState == TModifier::INSERT_) {
							++posOrig;
							++pos;
						}
						if (posOrig >= posOrigEnd)
							return false;
						_mod.errorPosOrig = posOrig;
						_mod.errorPos = pos;
						_mod.character = ValueSize<TValue>::VALUE - 1;
						_mod.skipChar = -1;
						++posOrig;
						erase(tmp, pos);
						break;

					case TModifier::INSERT_:
					default:
						// DELETE after INSERT is one SUBST (already enumerated)
						// eventually trim front SUBSTs
						if (lastState == TModifier::DELETE_ || (trim && posOrig == 0)) 
						{
							++posOrig;
							++pos;
						}
						if (posOrig > posOrigEnd)
							return false;
						_mod.errorPosOrig = posOrig;
						_mod.errorPos = pos;
						_mod.character = 0;
						_mod.skipChar = -1;
						insertValue(tmp, pos, (TValue)0);
						++pos;
						break;
				}
				lastState = _mod.state;
			}

			pos = length(tmp);
			bool cut = trim;
			for(unsigned i = 0; i < currentDistance; ++i) 
			{
				TModifier &_mod = mod[i];
				if (_mod.state != TModifier::DELETE_) {
					if (cut) {
						if (_mod.errorPos >= (TSignedSize)(pos - 1))
							return false;
						_mod.errorPosEnd = pos - 1;
					} else {
						if (_mod.errorPos >= (TSignedSize)pos)
							return false;
						_mod.errorPosEnd = pos;
					}
					--pos;
				} else {
					cut = false;
					if (_mod.errorPos > (TSignedSize)pos)
						return false;
					_mod.errorPosEnd = pos + 1;
				}
			}
			return true;
		}
	};


	template < typename TObject, unsigned DISTANCE >
    inline void
	goBegin(Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> & it)
	{
		typedef typename Value<TObject>::Type				TValue;
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize>	TModifier;

		if (empty(it.orig) || it.minDist > DISTANCE || it.minDist > length(it.orig)) {
			goEnd(it);
			return;
		}

		it.tmp = it.orig;
		for(unsigned i = 0; i <= DISTANCE; ++i) {
			it.mod[i].errorPosOrig = -1;
			it.mod[i].errorPos = -1;
			it.mod[i].errorPosEnd = -1;
			it.mod[i].character = ValueSize<TValue>::VALUE - 1;
			it.mod[i].state = TModifier::DISABLED_;
		}
		it.currentDistance = it.minDist;
		if (!it._reinit(0, 0))
			goEnd(it);
	}

	template < typename TObject, unsigned DISTANCE >
    inline void
	goEnd(Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> & it)
	{
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize>	TModifier;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier &mod = it.mod[i];
			mod.errorPos = -1;
			mod.character = 0;
			mod.state = TModifier::SUBST_;
		}
	}


	template < typename TObject, unsigned DISTANCE >
    inline Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> &
	operator ++ (Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> &it) 
	{
		typedef typename Value<TObject>::Type				TValue;
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize>	TModifier;
		typedef typename TModifier::TState					TState;

		// increment characters
		TModifier *mod = it.mod;
		do
		{
			// next replacement/insert value (loop core)
			if (++(mod->character) < ValueSize<TValue>::VALUE) {
				// output the original tuple only once
				if (mod->character == mod->skipChar) continue;
				assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);
				return it;
			}

			// reset counter
			if (mod->state != mod->DELETE_) {
				mod->character = (0 == mod->skipChar)? 1: 0;
				assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);
			}

			// next modifier
			++mod;
		} while (mod->state != TModifier::DISABLED_);

		// increment positions
		mod = it.mod;
		do
		{
			// restore char at old position
			if (mod->errorPos >= 0)
				assignValueAt(it.tmp, mod->errorPos, it.orig[mod->errorPosOrig]);

//					int iMax = (TSignedSize)(length(it.tmp) - i);
//					if (mod->state == mod->INSERT_) ++iMax;

			// next error position
			if (++(mod->errorPos) < mod->errorPosEnd)
			{
				++(mod->errorPosOrig);

				// set next char
				if (mod == it.mod)
				{	// for the first modifier we use an optimization
					if (mod->state != mod->DELETE_) {
						if (mod->state == mod->SUBST_)
							mod->skipChar = (unsigned) it.orig[mod->errorPosOrig];
						else
							mod->skipChar = -1;
						mod->character = (0 == mod->skipChar)? 1: 0;
						assignValueAt(it.tmp, mod->errorPos, (TValue) mod->character);
					}
				} else
					if (!it._reinit(mod->errorPos, mod->errorPosOrig))
						break;

				return it;
			}
			++mod;
		} while (mod->state != TModifier::DISABLED_);

		// increment states
		mod = it.mod;
		TModifier *modEnd = mod + DISTANCE;
		do 
		{
			// next edit state (subst->insert->delete)
			if (mod->state != mod->INSERT_) 
			{
				mod->state = (TState)(mod->state + 1);
				if (mod->state == TModifier::SUBST_)
					++it.currentDistance;
			
				if (!it._reinit(0, 0)) {
					mod = it.mod;
                    continue;						
				}
				return it;
			} else
				mod->state = mod->SUBST_;
			++mod;
		} while (mod != modEnd);

		// end
		for(mod = it.mod; mod != modEnd; ++mod) {
			mod->errorPos = -1;
			mod->character = 0;
			mod->state = mod->SUBST_;
		}
		return it;
    }

	template < typename TObject, unsigned DISTANCE >
	inline typename Size< Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> > >::Type
    length(Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> > const & me)
	{
		typedef typename Value<TObject>::Type TValue;
		typedef typename Size<TObject>::Type TSize;

		static const unsigned alpha = ValueSize<TValue>::VALUE;
		TSize sum = 0;
		TSize numerator = 1;
		TSize divisor = 1;
		TSize len = length(host(me));

		if (me.minDist == 0 && len > 0)
			++sum;

		if (me.minDist <= 1 && DISTANCE >= 1)
		{
			if (me.trim) {
				if (len > 2) sum += (alpha - 1) * (len - 2);	// substitutions
				sum += len;										// deletions
				if (len > 1) sum += alpha * (len - 1);			// inserts
			} else {
				sum += (alpha - 1) * len;						// substitutions
				sum += len;										// deletions
				sum += alpha * (len + 1);						// inserts
			}
		}

		if (me.minDist <= 2 && DISTANCE >= 2)
		{
			if (me.trim) {
				sum += (alpha  - 1) * (alpha - 1) *  len      * (len - 1) / 2;	// subst^2
				sum += (alpha  - 1)               * (len - 1) * (len - 1);		// subst*del
				sum += (alpha  - 1) *  alpha      *  len      * (len + 1);		// subst*ins
				sum +=                               len      * (len - 1) / 2;	// del^2
				sum +=  alpha       *  alpha      * (len + 1) * (len + 1) / 2;	// ins^2
			} else {
				sum += (alpha  - 1) * (alpha - 1) *  len      * (len - 1) / 2;	// subst^2
				sum += (alpha  - 1)               * (len - 1) * (len - 1);		// subst*del
				sum += (alpha  - 1) *  alpha      *  len      * (len + 1);		// subst*ins
				sum +=                               len      * (len - 1) / 2;	// del^2
				sum +=  alpha       *  alpha      * (len + 1) * (len + 1) / 2;	// ins^2
			}
		}

		// TODO: length function for DISTANCE >= 3 (if anyone needs should this)

		return sum;
    }

//////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// begin
	//////////////////////////////////////////////////////////////////////////////

	template < typename TObject, typename TSpec >
	inline Iter< Enumerator<TObject, TSpec>, Standard >
	begin(Enumerator<TObject, TSpec> &enumerator, Standard) {
		return Iter< Enumerator<TObject, TSpec>, Standard >(host(enumerator), enumerator.minDist, enumerator.trim);
	}
	template < typename TObject, typename TSpec >
	inline Iter< Enumerator<TObject, TSpec>, Standard >
	begin(Enumerator<TObject, TSpec> const &enumerator, Standard) {
		return Iter< Enumerator<TObject, TSpec>, Standard >(host(enumerator), enumerator.minDist, enumerator.trim);
	}

	//////////////////////////////////////////////////////////////////////////////
	// end
	//////////////////////////////////////////////////////////////////////////////

	template < typename TObject, typename TSpec >
	inline Iter< Enumerator<TObject, TSpec>, Standard >
	end(Enumerator<TObject, TSpec> &enumerator, Standard) {
		Iter< Enumerator<TObject, TSpec>, Standard > iter(host(enumerator), enumerator.minDist, enumerator.trim, MinimalCtor());
		goEnd(iter);
		return iter;
	}
	template < typename TObject, typename TSpec >
	inline Iter< Enumerator<TObject, TSpec>, Standard >
	end(Enumerator<TObject, TSpec> const &enumerator, Standard) {
		Iter< Enumerator<TObject, TSpec>, Standard > iter(host(enumerator), enumerator.minDist, enumerator.trim, MinimalCtor());
		goEnd(iter);
		return iter;
	}

	//////////////////////////////////////////////////////////////////////////////
	// operator ==
	//////////////////////////////////////////////////////////////////////////////

	template <typename TObject, unsigned DISTANCE>
	inline bool
	operator == (
		Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & a,
		Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & b)
	{
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		if (&a.orig != &b.orig) return false;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier const &modA = a.mod[i];
			TModifier const &modB = b.mod[i];
			if (modA.errorPos != modB.errorPos || modA.character != modB.character)
				return false;
		}

		return true;
	}

	template <typename TObject, unsigned DISTANCE>
	inline bool
	operator == (
		Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> const & a,
		Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> const & b)
	{
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize>	TModifier;

		if (&a.orig != &b.orig) return false;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier const &modA = a.mod[i];
			TModifier const &modB = b.mod[i];
			if (modA.errorPos != modB.errorPos || 
				modA.character != modB.character || 
				modA.state != modB.state)
				return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////////
	// operator !=
	//////////////////////////////////////////////////////////////////////////////

	template <typename TObject, unsigned DISTANCE>
	inline bool
	operator != (
		Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & a,
		Iter<Enumerator<TObject, EditEnvironment<HammingDistance, DISTANCE> >, Standard> const & b)
	{
		typedef typename Size<TObject>::Type			TSize;
		typedef typename MakeSigned_<TSize>::Type		TSignedSize;
		typedef EnumeratorHammingModifier_<TSignedSize>	TModifier;

		if (&a.orig != &b.orig) return true;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier const &modA = a.mod[i];
			TModifier const &modB = b.mod[i];
			if (modA.errorPos != modB.errorPos || modA.character != modB.character)
				return true;
		}

		return false;
	}

	template <typename TObject, unsigned DISTANCE>
	inline bool
	operator != (
		Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> const & a,
		Iter<Enumerator<TObject, EditEnvironment<LevenshteinDistance, DISTANCE> >, Standard> const & b)
	{
		typedef typename Size<TObject>::Type				TSize;
		typedef typename MakeSigned_<TSize>::Type			TSignedSize;
		typedef EnumeratorLevenshteinModifier_<TSignedSize>	TModifier;

		if (&a.orig != &b.orig) return true;

		for(unsigned i = 0; i < DISTANCE; ++i) {
			TModifier const &modA = a.mod[i];
			TModifier const &modB = b.mod[i];
			if (modA.errorPos != modB.errorPos || 
				modA.character != modB.character || 
				modA.state != modB.state)
				return true;
		}

		return false;
	}
}

#endif
