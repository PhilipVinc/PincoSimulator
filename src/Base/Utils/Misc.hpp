//
// Created by Filippo Vicentini on 30/04/2018.
//

#ifndef SIMULATOR_MISC_HPP
#define SIMULATOR_MISC_HPP
#include <memory>

template <typename To, typename From, typename Deleter>
    std::unique_ptr<To, Deleter> dynamic_unique_cast(std::unique_ptr<From, Deleter>&& p) {
        if (To* cast = dynamic_cast<To*>(p.get()))
        {
            std::unique_ptr<To, Deleter> result(cast, std::move(p.get_deleter()));
            p.release();
            return result;
        }
        return std::unique_ptr<To, Deleter>(nullptr); // or throw std::bad_cast() if you prefer
    }


#endif //SIMULATOR_MISC_HPP
