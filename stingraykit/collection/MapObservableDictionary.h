#ifndef STINGRAYKIT_COLLECTION_MAPOBSERVABLEDICTIONARY_H
#define STINGRAYKIT_COLLECTION_MAPOBSERVABLEDICTIONARY_H


#include <stingraykit/collection/IObservableDictionary.h>
#include <stingraykit/collection/MapDictionary.h>


namespace stingray
{

	/**
	 * @addtogroup toolkit_collections
	 * @{
	 */

	template < typename KeyType_, typename ValueType_, typename CompareType_ = std::less<KeyType_> >
	class MapObservableDictionary : public ObservableDictionaryWrapper<MapDictionary<KeyType_, ValueType_, CompareType_> >
	{ };

	/** @} */

}


#endif