//***************************************************************************
//* Copyright (c) 2012 Saint-Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//****************************************************************************

#include "blockfinder.h"

namespace SyntenyFinder
{
	const size_t BlockFinder::UNUSED = -1;

	namespace
	{
		bool CmpSizePair(const std::pair<size_t, size_t> & a, const std::pair<size_t, size_t> & b)
		{
			return a.first == b.first && a.second == b.second;
		}

		struct BifurcationMark
		{
			size_t bifId;
			size_t distance;
			BifurcationMark() {}
			BifurcationMark(size_t bifId, size_t distance): bifId(bifId),
				distance(distance) {}

			bool operator < (const BifurcationMark & compare) const
			{
				if(bifId != compare.bifId)
				{
					return bifId < compare.bifId;
				}

				return distance < compare.distance;
			}
		};		

		size_t MaxBifurcationMultiplicity(const BifurcationStorage & bifStorage,
			StrandIterator it, size_t distance)
		{
			size_t ret = 0;
			for(size_t i = 0; i < distance - 1; i++)
			{
				size_t bifId = bifStorage.GetBifurcation(++it);
				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					ret = std::max(ret, bifStorage.CountBifurcations(bifId));
				}
			}

			return ret;
		}

		void EraseBifurcations(DNASequence & sequence,
			BifurcationStorage & bifStorage,
			size_t k,
			const std::vector<StrandIterator> & startKMer,
			VisitData targetData,
			std::vector<std::pair<size_t, size_t> > & lookForward,
			std::vector<std::pair<size_t, size_t> > & lookBack)
		{
			lookBack.clear();
			lookForward.clear();
			StrandIterator amer = AdvanceForward(startKMer[targetData.kmerId], k).Invert();
			StrandIterator bmer = AdvanceForward(startKMer[targetData.kmerId], targetData.distance);
			for(size_t i = 0; i < k; i++, ++amer, ++bmer)
			{
				size_t bifId = bifStorage.GetBifurcation(amer);
				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					bifStorage.ErasePoint(amer);
					lookBack.push_back(std::make_pair(i, bifId));
				}

				bifId = bifStorage.GetBifurcation(bmer);
				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					bifStorage.ErasePoint(bmer);
					lookForward.push_back(std::make_pair(i, bifId));
				}
			}

			amer = startKMer[targetData.kmerId];
			bmer = AdvanceForward(amer, k + targetData.distance).Invert();
			for(size_t i = 0; i < k + targetData.distance; i++, ++amer, ++bmer)
			{
				bifStorage.ErasePoint(amer);
				bifStorage.ErasePoint(bmer);
			}
		}

		void UpdateBifurcations(DNASequence & sequence,
			BifurcationStorage & bifStorage,
			size_t k,
			const std::vector<StrandIterator> & startKMer,
			VisitData sourceData,
			VisitData targetData,
			const std::vector<std::pair<size_t, size_t> > & lookForward,
			const std::vector<std::pair<size_t, size_t> > & lookBack)
		{
			size_t anear = 0;
			size_t bnear = 0;
			StrandIterator amer = AdvanceForward(startKMer[targetData.kmerId], k).Invert();
			StrandIterator bmer = AdvanceForward(startKMer[targetData.kmerId], sourceData.distance);

			for(size_t i = 0; i < k; i++, ++amer, ++bmer)
			{
				if(anear < lookBack.size() && i == lookBack[anear].first)
				{
					bifStorage.AddPoint(amer, lookBack[anear++].second);
				}

				if(bnear < lookForward.size() && i == lookForward[bnear].first)
				{
					bifStorage.AddPoint(bmer, lookForward[bnear++].second);
				}
			}

			amer = startKMer[targetData.kmerId];
			bmer = AdvanceForward(startKMer[targetData.kmerId], sourceData.distance + k).Invert();
			StrandIterator srcAMer = startKMer[sourceData.kmerId];
			StrandIterator srcBMer = AdvanceForward(startKMer[sourceData.kmerId], sourceData.distance + k).Invert();
			for(size_t i = 0; i < sourceData.distance + 1; i++, ++amer, ++bmer, ++srcAMer, ++srcBMer)
			{
				std::cout << *amer;
				size_t bifId = bifStorage.GetBifurcation(srcAMer);
				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					bifStorage.AddPoint(amer, bifId);
				}

				bifId = bifStorage.GetBifurcation(srcBMer);
				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					bifStorage.AddPoint(bmer, bifId);
				}
			}

			std::cout << std::endl;
		}

		bool Overlap(size_t k,
			const std::vector<StrandIterator> & startKMer,
			VisitData sourceData,
			VisitData targetData)
		{
			std::vector<size_t> occur;
			StrandIterator it = startKMer[sourceData.kmerId];
			for(size_t i = 0; i < sourceData.distance + k; i++, ++it)
			{
				occur.push_back(it.GetElementId());
			}

			it = startKMer[targetData.kmerId];
			std::sort(occur.begin(), occur.end());
			for(size_t i = 0; i < targetData.distance + k; i++, ++it)
			{
				if(std::binary_search(occur.begin(), occur.end(), it.GetElementId()))
				{
					return true;
				}
			}

			return false;
		}
		
		void FillVisit(const DNASequence & sequence,
			const BifurcationStorage & bifStorage, 
			StrandIterator kmer,
			size_t minBranchSize,
			std::vector<BifurcationMark> & visit)			
		{
			visit.clear();
			size_t start = bifStorage.GetBifurcation(kmer);
			++kmer;
			for(size_t step = 1; step < minBranchSize && kmer.AtValidPosition(); ++kmer, step++)
			{
				size_t bifId = bifStorage.GetBifurcation(kmer);
				if(bifId == start)
				{
					break;
				}

				if(bifId != BifurcationStorage::NO_BIFURCATION)
				{
					visit.push_back(BifurcationMark(bifId, step));
				}
			}

			std::sort(visit.begin(), visit.end());
		}

		void SpellBulges(const DNASequence & sequence, size_t k,
			size_t bifStart,
			size_t bifEnd,
			const std::vector<StrandIterator> & startKMer,
			const std::vector<VisitData> & visitData)
		{
			static size_t bulge = 0;
			std::cerr << "Bulge #" << bulge++ << ", " << "(" << bifStart << ", " << bifEnd << ")" << std::endl;
			for(size_t i = 0; i < visitData.size(); i++)
			{
				std::cerr << "Branch #" << i << ", size = " << visitData[i].distance + k << ":" << std::endl;			
				BlockFinder::PrintPath(sequence, startKMer[visitData[i].kmerId], k, visitData[i].distance, std::cerr);
			}

			std::cerr << DELIMITER << std::endl;
		}

	}

	void BlockFinder::NotifyBefore(NotificationData data, PositiveIterator begin, PositiveIterator end)
	{
		DNASequence::SequenceNegIterator rbegin(end);
		DNASequence::SequenceNegIterator rend(begin);
		data.bifStorage->NotifyBefore(begin, end);
		SelectInvalid<PositiveIterator, &BlockFinder::posInvalid>(data, begin, end, DNASequence::positive);
		SelectInvalid<NegativeIterator, &BlockFinder::negInvalid>(data, rbegin, rend, DNASequence::negative);
	}

	void BlockFinder::NotifyAfter(NotificationData data, PositiveIterator begin, PositiveIterator end)
	{
		DNASequence::SequenceNegIterator rbegin(end);
		DNASequence::SequenceNegIterator rend(begin);
		data.bifStorage->NotifyAfter(begin, end);
		AddInvalid<PositiveIterator, &BlockFinder::posInvalid>(data, begin, end, DNASequence::positive);
		AddInvalid<NegativeIterator, &BlockFinder::negInvalid>(data, rbegin, rend, DNASequence::negative);
	}
	
	void BlockFinder::AddRestricted(RestrictionMap & restricted, StrandIterator it, size_t index, size_t k)
	{
		for(size_t j = 0; j < k; j++, ++it)
		{
			restricted.insert(std::make_pair(it.GetElementId(), index));
		}
	}

	void BlockFinder::RemoveRestricted(RestrictionMap & restricted, StrandIterator it, size_t index, size_t k)
	{
		for(size_t j = 0; j < k; j++, ++it)
		{
			typedef RestrictionMap::iterator RIterator;
			std::pair<RIterator, RIterator> range = restricted.equal_range(it.GetElementId());
			RIterator toErase = std::find_if(range.first, range.second, boost::bind(CmpSizePair, _1, std::make_pair(it.GetElementId(), index)));
			assert(toErase != restricted.end());
			restricted.erase(toErase);
		}
	}

	void BlockFinder::CollapseBulgeGreedily(DNASequence & sequence,
		BifurcationStorage & bifStorage,
		size_t k,
		NotificationData notification,
		VisitData sourceData,
		VisitData targetData)
	{
		std::vector<StrandIterator> & startKMer = *notification.startKMer;
		RestrictionMap & restricted = *notification.restricted;

	#ifdef _DEBUG
		static size_t bulge = 0;
		std::cerr << "Bulge #" << bulge++ << std::endl;
		std::cerr << "Before: " << std::endl;
		BlockFinder::PrintRaw(sequence, std::cerr);
		std::cerr << "Source branch: " << std::endl;			
		BlockFinder::PrintPath(sequence, startKMer[sourceData.kmerId], k, sourceData.distance, std::cerr);
		std::cerr << "Target branch: " << std::endl;			
		BlockFinder::PrintPath(sequence, startKMer[targetData.kmerId], k, targetData.distance, std::cerr);
		bifStorage.Dump(sequence, k, std::cerr);
	#endif
			
		std::vector<BifurcationMark> v;
		FillVisit(sequence, bifStorage, startKMer[sourceData.kmerId], sourceData.distance, v);
		StrandIterator it = startKMer[targetData.kmerId];
		for(size_t step = 0; step < targetData.distance + k; step++, ++it)
		{
			typedef RestrictionMap::const_iterator MMIterator;
			std::pair<MMIterator, MMIterator> kt = restricted.equal_range(it.GetElementId());
			for(; kt.first != kt.second; ++kt.first)
			{
				if(kt.first->second != targetData.kmerId)
				{
					startKMer[kt.first->second] = sequence.PositiveEnd(0);
					notification.iteratorIndex->erase(startKMer[kt.first->second]);
				}
			}
		}

		std::vector<std::pair<size_t, size_t> > lookForward;
		std::vector<std::pair<size_t, size_t> > lookBack;
		EraseBifurcations(sequence, bifStorage, k, startKMer, targetData, lookForward, lookBack);
		StrandIterator sourceIt = startKMer[sourceData.kmerId];
		StrandIterator targetIt = startKMer[targetData.kmerId];			
		sequence.Replace(AdvanceForward(sourceIt, k),
			sourceData.distance,
			AdvanceForward(targetIt, k),
			targetData.distance,
			boost::bind(&BlockFinder::NotifyBefore, boost::ref(*this), notification, _1, _2),
			boost::bind(&BlockFinder::NotifyAfter, boost::ref(*this), notification, _1, _2));
		UpdateBifurcations(sequence, bifStorage, k, startKMer, sourceData, targetData, lookForward, lookBack);

	#ifdef _DEBUG
		std::cerr << "After: " << std::endl;
		BlockFinder::PrintRaw(sequence, std::cerr);
		std::cerr << "Source branch: " << std::endl;			
		BlockFinder::PrintPath(sequence, startKMer[sourceData.kmerId], k, sourceData.distance, std::cerr);
		std::cerr << "Target branch: " << std::endl;			
		BlockFinder::PrintPath(sequence, startKMer[targetData.kmerId], k, sourceData.distance, std::cerr);
		bifStorage.Dump(sequence, k, std::cerr);
		std::cerr << DELIMITER << std::endl;
		Test(sequence, bifStorage, k);
	#endif
	}		

	size_t BlockFinder::RemoveBulges(DNASequence & sequence,
		BifurcationStorage & bifStorage, size_t k, size_t minBranchSize, size_t bifId)
	{	
		size_t ret = 0;	
		IteratorVector startKMer;
		RestrictionMap restricted;
		IteratorIndexMap iteratorIndex;
		NotificationData notification(&restricted, &iteratorIndex, &startKMer, &bifStorage, k);
		if(bifStorage.ListPositions(bifId, std::back_inserter(startKMer)) < 2)
		{
			return ret;
		}

		std::vector<char> endChar(startKMer.size(), ' ');
		for(size_t i = 0; i < startKMer.size(); i++)
		{
			if(ProperKMer(startKMer[i], k + 1))
			{                    
				endChar[i] = *AdvanceForward(startKMer[i], k);
			}

			iteratorIndex[startKMer[i]] = i;
			AddRestricted(restricted, startKMer[i], i, k);			
		}
		
		std::vector<BifurcationMark> visit;
		for(size_t kmerI = 0; kmerI < startKMer.size(); kmerI++)
		{
			if(!startKMer[kmerI].AtValidPosition())
			{
				continue;
			}

			FillVisit(sequence, bifStorage, startKMer[kmerI], minBranchSize, visit);
			for(size_t kmerJ = kmerI + 1; kmerJ < startKMer.size(); kmerJ++)
			{
				if(!startKMer[kmerJ].AtValidPosition() || endChar[kmerI] == endChar[kmerJ])
				{
					continue;
				}

				StrandIterator kmer = ++StrandIterator(startKMer[kmerJ]);
				for(size_t step = 1; kmer.AtValidPosition() && step < minBranchSize; ++kmer, step++)
				{
					size_t nowBif = bifStorage.GetBifurcation(kmer);
					if(nowBif != BifurcationStorage::NO_BIFURCATION)
					{
						if(nowBif == bifId)
						{
							break;
						}

						std::vector<BifurcationMark>::iterator vt = std::lower_bound(visit.begin(), visit.end(), BifurcationMark(nowBif, 0));
						if(vt != visit.end() && vt->bifId == nowBif)
						{
							VisitData jdata(kmerJ, step);
							VisitData idata(kmerI, vt->distance);
							if(Overlap(k, startKMer, idata, jdata) || nowBif == bifId)
							{
								break;
							}

							++ret;
							size_t imlp = MaxBifurcationMultiplicity(bifStorage, startKMer[kmerI], idata.distance);
							size_t jmlp = MaxBifurcationMultiplicity(bifStorage, startKMer[kmerJ], jdata.distance);
							bool iless = imlp > jmlp || (imlp == jmlp && idata.kmerId < jdata.kmerId);						
							if(iless)
							{
								endChar[jdata.kmerId] = endChar[idata.kmerId];
								CollapseBulgeGreedily(sequence, bifStorage, k, notification, idata, jdata);
							}
							else
							{
								endChar[idata.kmerId] = endChar[jdata.kmerId];
								CollapseBulgeGreedily(sequence, bifStorage, k, notification, jdata, idata);
								FillVisit(sequence, bifStorage, startKMer[kmerI], minBranchSize, visit);
							}

							break;
						}
					}
				}
			}
		}

		return ret;
	}
}