#include <cubos/core/log.hpp>
#include <cubos/core/reflection/traits/dictionary.hpp>

using cubos::core::reflection::DictionaryTrait;
using cubos::core::reflection::Type;

DictionaryTrait::DictionaryTrait(const Type& keyType, const Type& valueType, Length length, Begin begin, Find find,
                                 Advance advance, Stop stop, Key key, Value value)
    : mKeyType(keyType)
    , mValueType(valueType)
    , mLength(length)
    , mBegin(begin)
    , mFind(find)
    , mAdvance(advance)
    , mStop(stop)
    , mKey(key)
    , mValue(value)
    , mInsertDefault(nullptr)
    , mInsertCopy(nullptr)
    , mInsertMove(nullptr)
    , mErase(nullptr)
{
}

void DictionaryTrait::setInsertDefault(InsertDefault insertDefault)
{
    CUBOS_ASSERT(!mInsertDefault, "Insert default already set");
    mInsertDefault = insertDefault;
}

void DictionaryTrait::setInsertCopy(InsertCopy insertCopy)
{
    CUBOS_ASSERT(!mInsertCopy, "Insert copy already set");
    mInsertCopy = insertCopy;
}

void DictionaryTrait::setInsertMove(InsertMove insertMove)
{
    CUBOS_ASSERT(!mInsertMove, "Insert move already set");
    mInsertMove = insertMove;
}

void DictionaryTrait::setErase(Erase erase)
{
    CUBOS_ASSERT(!mErase, "Erase already set");
    mErase = erase;
}

const Type& DictionaryTrait::keyType() const
{
    return mKeyType;
}

const Type& DictionaryTrait::valueType() const
{
    return mValueType;
}

std::size_t DictionaryTrait::length(const void* instance) const
{
    return mLength(instance);
}

DictionaryTrait::Iterator DictionaryTrait::begin(void* instance) const
{
    return Iterator{mBegin(reinterpret_cast<uintptr_t>(instance), true), instance, *this};
}

DictionaryTrait::ConstIterator DictionaryTrait::begin(const void* instance) const
{
    return ConstIterator{mBegin(reinterpret_cast<uintptr_t>(instance), false), instance, *this};
}

DictionaryTrait::Iterator DictionaryTrait::find(void* instance, const void* key) const
{
    return Iterator{mFind(reinterpret_cast<uintptr_t>(instance), key, true), instance, *this};
}

DictionaryTrait::ConstIterator DictionaryTrait::find(const void* instance, const void* key) const
{
    return ConstIterator{mFind(reinterpret_cast<uintptr_t>(instance), key, false), instance, *this};
}

bool DictionaryTrait::insertDefault(void* instance, const void* key) const
{
    if (mInsertDefault != nullptr)
    {
        mInsertDefault(instance, key);
        return true;
    }

    return false;
}

bool DictionaryTrait::insertCopy(void* instance, const void* key, const void* value) const
{
    if (mInsertCopy != nullptr)
    {
        mInsertCopy(instance, key, value);
        return true;
    }

    return false;
}

bool DictionaryTrait::insertMove(void* instance, const void* key, void* value) const
{
    if (mInsertMove != nullptr)
    {
        mInsertMove(instance, key, value);
        return true;
    }

    return false;
}

bool DictionaryTrait::erase(void* instance, Iterator& iterator) const
{
    CUBOS_ASSERT(!iterator.isNull(), "Cannot erase null iterator");

    if (mErase != nullptr)
    {
        mErase(instance, iterator.mInner, true);
        mStop(iterator.mInner, true);
        iterator.mInner = nullptr;
        return true;
    }

    return false;
}

bool DictionaryTrait::erase(void* instance, ConstIterator& iterator) const
{
    CUBOS_ASSERT(!iterator.isNull(), "Cannot erase null iterator");

    if (mErase != nullptr)
    {
        mErase(instance, iterator.mInner, false);
        mStop(iterator.mInner, false);
        iterator.mInner = nullptr;
        return true;
    }

    return false;
}

bool DictionaryTrait::hasInsertDefault() const
{
    return mInsertDefault != nullptr;
}

bool DictionaryTrait::hasInsertCopy() const
{
    return mInsertCopy != nullptr;
}

bool DictionaryTrait::hasInsertMove() const
{
    return mInsertMove != nullptr;
}

bool DictionaryTrait::hasErase() const
{
    return mErase != nullptr;
}

DictionaryTrait::Iterator::~Iterator()
{
    if (mInner != nullptr)
    {
        mTrait.mStop(mInner, true);
    }
}

DictionaryTrait::Iterator::Iterator(void* inner, void* instance, const DictionaryTrait& trait)
    : mInner(inner)
    , mInstance(instance)
    , mTrait(trait)
{
}

DictionaryTrait::Iterator::Iterator(const Iterator& other)
    : mInner(nullptr)
    , mInstance(other.mInstance)
    , mTrait(other.mTrait)
{
    if (!other.isNull())
    {
        mInner = mTrait.mFind(reinterpret_cast<uintptr_t>(mInstance), other.key(), true);
    }
}

DictionaryTrait::Iterator::Iterator(Iterator&& other)
 noexcept     : mInner(other.mInner)
    , mInstance(other.mInstance)
    , mTrait(other.mTrait)
{
    other.mInner = nullptr;
}

const void* DictionaryTrait::Iterator::key() const
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot get key from null iterator");
    return mTrait.mKey(mInner, true);
}

void* DictionaryTrait::Iterator::value() const
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot get value from null iterator");
    return reinterpret_cast<void*>(mTrait.mValue(mInner, true));
}

bool DictionaryTrait::Iterator::advance()
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot advance null iterator");

    if (mTrait.mAdvance(reinterpret_cast<uintptr_t>(mInstance), mInner, true))
    {
        return true;
    }

    mTrait.mStop(mInner, true);
    mInner = nullptr;
    return false;
}

bool DictionaryTrait::Iterator::isNull() const
{
    return mInner == nullptr;
}

DictionaryTrait::ConstIterator::~ConstIterator()
{
    if (mInner != nullptr)
    {
        mTrait.mStop(mInner, false);
    }
}

DictionaryTrait::ConstIterator::ConstIterator(void* inner, const void* instance, const DictionaryTrait& trait)
    : mInner(inner)
    , mInstance(instance)
    , mTrait(trait)
{
}

DictionaryTrait::ConstIterator::ConstIterator(const ConstIterator& other)
    : mInner(nullptr)
    , mInstance(other.mInstance)
    , mTrait(other.mTrait)
{
    if (!other.isNull())
    {
        mInner = mTrait.mFind(reinterpret_cast<uintptr_t>(mInstance), other.key(), false);
    }
}

DictionaryTrait::ConstIterator::ConstIterator(ConstIterator&& other)
 noexcept     : mInner(other.mInner)
    , mInstance(other.mInstance)
    , mTrait(other.mTrait)
{
    other.mInner = nullptr;
}

const void* DictionaryTrait::ConstIterator::key() const
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot get key from null iterator");
    return mTrait.mKey(mInner, false);
}

const void* DictionaryTrait::ConstIterator::value() const
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot get value from null iterator");
    return reinterpret_cast<void*>(mTrait.mValue(mInner, false));
}

bool DictionaryTrait::ConstIterator::advance()
{
    CUBOS_ASSERT(mInner != nullptr, "Cannot advance null iterator");

    if (mTrait.mAdvance(reinterpret_cast<uintptr_t>(mInstance), mInner, false))
    {
        return true;
    }

    mTrait.mStop(mInner, false);
    mInner = nullptr;
    return false;
}

bool DictionaryTrait::ConstIterator::isNull() const
{
    return mInner == nullptr;
}
