//////////////////
// Generated by makedp(1.0.0.0)
////////////////


///////////////////////////////
/////////////////// stLoginAppearance
/////////////////////////////////
template <Dp::SizeType _Sz>
bool load_some(Dp::Reader &r,Dp::Array<stLoginAppearance,_Sz> &elems) 
{
	Dp::SizeType count; r >> count;
	Dp::Array<uint8_t,32> flags; r >> flags;

	for (Dp::SizeType i=0; i<count; ++i)
	{
		stLoginAppearance elem;

		Dp::SizeType structLen; r >> structLen;
		Dp::SizeType structEnd = r.cursor() + structLen;

		//----- userId -----
		r >> elem.userId;
		//----- nick -----
		r >> elem.nick;
		//----- photoIndex -----
		r >> elem.photoIndex;
		//----- grade -----
		r >> elem.grade;
		//----- redTime -----
		r >> elem.redTime;
		//----- mood -----
		r >> elem.mood;
		//----- headUrl -----
		r >> elem.headUrl;
		//----- sex -----
		r >> elem.sex;
		//----- charm -----
		r >> elem.charm;
		//----- wealth -----
		r >> elem.wealth;
		//----- activity -----
		r >> elem.activity;
		//----- vipRoomId -----
		r >> elem.vipRoomId;
		//----- tycoon -----
		r >> elem.tycoon;
		//----- seller -----
		r >> elem.seller;
		//----- weekStar -----
		r >> elem.weekStar;
		if (!r.ok()) return false;

		for (Dp::SizeType j=0; j<flags.size(); ++j) 
		{
			uint8_t mid = Dp::memberId(flags[j]);
			uint8_t msize  = Dp::memberSize((flags[j]));

			switch (mid)
			{
			default:
				switch (msize)
				{
				case Dp::TypesEnum::size_8: r.skip(1); break;
				case Dp::TypesEnum::size_16:r.skip(2); break;
				case Dp::TypesEnum::size_32:r.skip(4); break;
				case Dp::TypesEnum::size_64:r.skip(8); break;
				case Dp::TypesEnum::size_array: { Dp::SizeType as; r >> as; r.skip(as); } break;
				default:break;
				}
				break;
			}
		}

		if (!r.ok() || r.cursor()!=structEnd) 
			return false;
		elems.push_back(elem);
	}
	return true;
}
template <uint32_t _Sz>
void save_some(Dp::Writer &w,const Dp::Array<stLoginAppearance,_Sz> &elems) 
{
	w << elems.size();

	w << (Dp::SizeType)0;

	for (size_t i=0; i<elems.size(); ++i)
	{
		const stLoginAppearance &elem = elems[i];

		Dp::SizeType start = w.cursor();
		w.skip(sizeof(Dp::SizeType));

		//----- elem.userId -----
		w << elem.userId;
		//----- elem.nick -----
		w << elem.nick;
		//----- elem.photoIndex -----
		w << elem.photoIndex;
		//----- elem.grade -----
		w << elem.grade;
		//----- elem.redTime -----
		w << elem.redTime;
		//----- elem.mood -----
		w << elem.mood;
		//----- elem.headUrl -----
		w << elem.headUrl;
		//----- elem.sex -----
		w << elem.sex;
		//----- elem.charm -----
		w << elem.charm;
		//----- elem.wealth -----
		w << elem.wealth;
		//----- elem.activity -----
		w << elem.activity;
		//----- elem.vipRoomId -----
		w << elem.vipRoomId;
		//----- elem.tycoon -----
		w << elem.tycoon;
		//----- elem.seller -----
		w << elem.seller;
		//----- elem.weekStar -----
		w << elem.weekStar;
		Dp::SizeType len = w.cursor() - start - sizeof(Dp::SizeType);
		w.set(start,len);
	}
}
