#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <tuple>
#include <cassert>

namespace k3d 
{

	/**
	 * TResource must implement method "Release"
	 * TResourceUID is the key of the resource
	 * Need Memory Budget ?
	 */
	template <class TResource, typename TResourceUID, typename TResourceSizer>
	class TResourceManager {
	public:
		using TPtrResource = std::shared_ptr<TResource>;

		TResourceManager() {}
		~TResourceManager() {}

		void Insert(TResourceUID uid, TPtrResource pRes) {
			assert(pRes.get()!=nullptr);
			//std::mutex
			if (m_ResMap.find(uid) == m_ResMap.end()) {
				m_ResMap[uid] = std::make_tuple<TPtrResource, TResourceSizer>(pRes, pRes->GetMemUsage());
			}
		}

		TPtrResource Get(TResourceUID uid) {
			return m_ResMap[uid].get<0>();
		}

		void Delete(TResourceUID uid) {
			if (m_ResMap.find(uid) != m_ResMap.end()) {
				m_ResMap[uid]->Release();
				m_ResMap.erase(uid);
			}
		}

	private:
		std::unordered_map<
			TResourceUID, 
			std::tuple<TPtrResource, TResourceSizer>
		>	m_ResMap;
	};
}