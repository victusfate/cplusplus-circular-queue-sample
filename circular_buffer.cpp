#include <algorithm>
#include <numeric>
#include <vector>
#include <iterator>
#include <set>
#include <map>
#include <boost/circular_buffer.hpp>

#include <iostream>

using namespace std;
using namespace boost;

typedef std::pair<long,long>	TrackID;

class DwellReport { 
public:
	bool exceedsDiscrimThresh;
	TrackID mTrackID;
};


class DiscrimMofN {
	public:
		DiscrimMofN(long M,long N) : mM(M),mN(N) {}
		
		virtual ~DiscrimMofN() {
			map<TrackID,circular_buffer<long> *>::iterator ptr = mHistory.begin();
			for (; ptr != mHistory.end();ptr++) {
				if (ptr->second) delete ptr->second;
			}
		};

		void Update(vector<DwellReport> &Dwells) {
			mCurDiscrim.clear();

			// identify tracks discriminated this frame
			for (long i=0;i < Dwells.size();i++) {
				const DwellReport &det = Dwells[i];
				set<TrackID>::iterator done = mD.find(det.mTrackID);
				if (done == mD.end()) { // if not at end, already discrim'd
					if (det.exceedsDiscrimThresh) mCurDiscrim[det.mTrackID] = 1; // if ever discrim'd set it to 1
					else {
						map<TrackID,long>::iterator j = mCurDiscrim.find(det.mTrackID);
						if (j == mCurDiscrim.end()) { // else it was discrim'd by another dwell or is already zeroed
							mCurDiscrim[det.mTrackID] = 0; // set it to zero if it doesn't exist
						}
					}
				}
			}
			// pushback 1 for discrimated tracks, 0 for dwell but no discrim (not detected or not discriminated), nothing for not observed
			map<TrackID,long>::iterator j = mCurDiscrim.begin();
			for (; j != mCurDiscrim.end();j++) {
				set<TrackID>::iterator done = mD.find(j->first);
				if (done == mD.end()) { // if not at end, already discrim'd
					map<TrackID,circular_buffer<long> *>::iterator h = mHistory.find(j->first);
					circular_buffer<long> *buf;
					if (h == mHistory.end()) { // doesn't exist in history yet, allocate buf
						buf = new circular_buffer<long>(mN);
						mHistory[j->first] = buf;
					}
					else buf = h->second;
					buf->push_back(j->second);
					long sum = std::accumulate( buf->begin(), buf->end(), 0 );
						cout << "sum M for " << j->first.first << "," << j->first.second << " is " << sum << endl;
					if (sum >= mM) {
						mD.insert(j->first);
						cout << "first M/N discrim satisfied for " << j->first.first << "," << j->first.second << endl;
					}
				}
			}
		}

		// members
		long mM;
		long mN;
		map<TrackID,circular_buffer<long> *> mHistory;
		map<TrackID,long> mCurDiscrim;
		set<TrackID> mD; // list of discrimated
};

int main( int argc, char** argv )
{	
	long M = 2;
	long N = 2;
	long NFrames = 5,NObjects = 10;
	srand(0);

	DiscrimMofN disc(M,N);
	for (long i=0;i < NFrames;i++) {
		vector<DwellReport> reports;
		cout << "DwellID\tExceedsDiscrim\n";
		for (long j=0;j < NObjects;j++) {
			TrackID temp(0,rand() % 3);
			DwellReport dr;
			dr.mTrackID = temp;
			dr.exceedsDiscrimThresh = rand() % 2; // if it's even DiscrimThresh
			bool Dwell = rand() % 2; // if it's even no dwell
			if (Dwell) {
				reports.push_back(dr);
				cout << dr.mTrackID.first << "," << dr.mTrackID.second << "\t" << dr.exceedsDiscrimThresh << endl;
			}
		}
		cout << endl;
		disc.Update(reports);
		cout << endl;
	}

}
