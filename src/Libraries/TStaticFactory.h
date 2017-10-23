//
//  TStaticFactory.h
//  TWMC++
//
//  Created by Filippo Vicentini on 22/10/17.
//  Copyright © 2017 Filippo Vicentini. All rights reserved.
//

#ifndef TStaticFactory_h
#define TStaticFactory_h

#pragma once

#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>

namespace Base
{
	template<typename T>
	typename T::first_type select1st_factory(const T& aValue)
	{
		return aValue.first;
	}

	template <typename KeyT, typename BaseT, typename... Args>
	class TFactory
	{
	public:
		using CompareT = std::less<KeyT>;
		typedef std::unique_ptr<BaseT> UniquePtr_t;
		typedef std::shared_ptr<BaseT> SharedPtr_t;
		typedef KeyT keyType;

	private:
		class FactoryBase
		{
		public:
			typedef std::unique_ptr<FactoryBase> BaseUniquePtr_t;

		public:
			FactoryBase(void) = default;

			virtual ~FactoryBase(void) = default;

			virtual BaseT* makeRawNewInstance(Args... data) const = 0;
			virtual UniquePtr_t makeUniqueNewInstance(Args... data) const = 0;
			virtual SharedPtr_t makeSharedNewInstance(Args... data) const = 0;
		};
		template <typename TypeT>
		class TFactoryInternal : public FactoryBase
		{
		public:
			using constrFunc = std::function<TypeT(Args...)>;

			TFactoryInternal(void)
					: FactoryBase()
			{
			}

			virtual BaseT* makeRawNewInstance(Args... data) const
			{
				return new TypeT(data...);
			}
			virtual UniquePtr_t makeUniqueNewInstance(Args... data) const
			{
				return std::make_unique<TypeT>(data...);
			}
			virtual SharedPtr_t makeSharedNewInstance(Args... data) const
			{
				return std::make_shared<TypeT>(data...);
			}
		};

	private:
		typedef std::map<KeyT, typename FactoryBase::BaseUniquePtr_t, CompareT> FactoryStringMap_t;

	public:
		TFactory(void)
		{
		}
		static FactoryStringMap_t* mFactoryMap()
		{
			static FactoryStringMap_t* bm = new FactoryStringMap_t;
			return bm;
		}

		template <typename TypeT>
		static void registerPlugin(const KeyT& aName)
		{
			static FactoryStringMap_t* bm = mFactoryMap();
			bm->insert(std::make_pair(aName, std::make_unique<TFactoryInternal<TypeT>>()));
			return;
		}

		static BaseT* makeRawNewInstance(const KeyT& aName, Args ...data)
		{
			static FactoryStringMap_t* bm = mFactoryMap();

			typename FactoryStringMap_t::const_iterator itFactory = bm->find(aName);
			if (itFactory == bm->end())
				return nullptr;
			return itFactory->second->makeRawNewInstance(data...);
		}

		static UniquePtr_t makeUniqueNewInstance(const KeyT& aName, Args ...data)
		{
			static FactoryStringMap_t* bm = mFactoryMap();

			typename FactoryStringMap_t::const_iterator itFactory = bm->find(aName);
			if (itFactory == bm->end())
				return nullptr;
			return itFactory->second->makeUniqueNewInstance(data...);
		}

		static SharedPtr_t makeSharedNewInstance(const KeyT& aName, Args ...data)
		{
			static FactoryStringMap_t* bm = mFactoryMap();

			typename FactoryStringMap_t::const_iterator itFactory = bm->find(aName);
			if (itFactory == bm->end())
				return nullptr;
			return itFactory->second->makeSharedNewInstance(data...);
		}

		static void getKeys(std::vector<KeyT>& aKeysResult)
		{
			static FactoryStringMap_t* bm = mFactoryMap();

			aKeysResult.resize(bm->size());
			std::transform(bm->begin(), bm->end(), aKeysResult.begin(), select1st_factory<typename FactoryStringMap_t::value_type>);
		}

		template <typename TypeT>
		class Registrator
		{
		public:
			Registrator(const KeyT& name)
			{
				registerPlugin<TypeT>(name);
			}
		};


	private:
		TFactory(const TFactory& aCopy);
		TFactory& operator=(const TFactory& aCopy);
	};

};

#endif /* TStaticFactory_h */
