////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5COMPTYPE_H
#define CPH5COMPTYPE_H


#ifdef _MSC_VER
#pragma warning(disable: 4355 4706 4115 4100 4201 4214 4054 4244 4267)
#endif /* _MSC_VER */


#include "H5Cpp.h"
#include "cph5utilities.h"

#include <sstream>

//TODO add support for assignment operator based copying of dynamic data.



class CPH5CompMemberArrayBase;

/*!
 * \brief The CPH5CompMemberBase class is a pure interface class to describe an
 *        individual member of a compound data type.
 * 
 * Class is very tightly coupled with CPH5CompMember. This  class allows a
 * container for compound member types to  maintain a list of children despite
 * each child being instantiated with a different template type.
 */
class CPH5CompMemberBase : public CPH5TreeNode
{
public:
    virtual ~CPH5CompMemberBase() {}
    virtual std::string getName() const = 0;
    virtual H5::DataType getType() const = 0;
    virtual int getSize() const = 0;
    virtual void latchAndMove(char *&ptr) = 0;
    virtual void latchAndMoveWithSwap(char *&ptr) = 0;
    virtual void copyAndMove(char *&ptr) const = 0;
    virtual std::string getStrOfValue() = 0;
    virtual void setArrayParent(CPH5CompMemberArrayBase *pArrParent) = 0;
};


/*!
 * \brief The CPH5CompMemberArrayBase class is a pass-through of the 
 *        CPH5CompMemberBase class, inherited only by CompMemberArray
 *        implementations to allow for type checking using
 *        dynamic_cast.
 */
class CPH5CompMemberArrayBase : public CPH5CompMemberBase
{
public:
    virtual ~CPH5CompMemberArrayBase() {}
    virtual std::string getName() const = 0;
    virtual H5::DataType getType() const = 0;
    virtual int getSize() const = 0;
    virtual void latchAndMove(char *&ptr) = 0;
    virtual void latchAndMoveWithSwap(char *&ptr) = 0;
    virtual void copyAndMove(char *&ptr) const = 0;
    virtual std::string getStrOfValue() = 0;
    virtual void setArrayParent(CPH5CompMemberArrayBase *pArrParent) = 0;
    virtual CPH5IOFacility *getIoFacility() = 0;
    virtual void signalChange() = 0;
};


// Forward declaration
class CPH5CompType;


/*!
 * \brief The CPH5CompMemberBaseThru class inherits the CompMemberBase class
 *        and passes its interface through, but allows for specialization based
 *        on whether the compound member is itself a compound type, similar to
 *        the Dataset class. This is the non-compound specialization.
 */
template<class T, const int I>
class CPH5CompMemberBaseThru : public CPH5CompMemberBase
{
    // NON-INHERITED SPECIALIZATION
public:
    
    /*!
     * \brief Constructor. Since this is a non-compound type we optionally
     *        accept the H5::DataType to assign to this compound member in
     *        the target HDF5 file. 
     * \param parent The CPH5CompType object that this member belongs to.
     * \param name The name of this compound member visible in the target HDF5
     *        file.
     * \param type The H5::DataType to assign to this compound member in the
     *        target HDF5 file.
     */
    CPH5CompMemberBaseThru(CPH5CompType *parent,
                           std::string name,
                           H5::DataType type)
        : mpParent(parent),
          mName(name),
          mpArrParent(0)
    {
        mType = type;
    }
    
    //TODO fix the constructor dependency issue in the inherited specialization
    // as well.
    /*!
     * \brief Constructor. This constructor does not accept an H5::DataType
     *        to assign to this compound member in the target HDF5 file.
     *        Instead, it uses the CPH5TypeProxy mappings from CPH5Utilities.
     * \param parent The CPH5CompType object that this member belongs to.
     * \param name The name of this compound member visible in the target HDF5
     *        file.
     */
    CPH5CompMemberBaseThru(CPH5CompType *parent,
                           std::string name)
        : mpParent(parent),
          mName(name),
          mpArrParent(0)
    {
        mType = static_cast<H5::DataType>(CPH5TypeProxy<T>());
    }
    
    
    /*!
     * \brief Empty constructor for if a standalone compound member object is
     *        desired.
     */
    CPH5CompMemberBaseThru()
        : mpParent(0),
          mpArrParent(0)
    {} // NOOP standalone object
    
    
    CPH5CompMemberBaseThru(const CPH5CompMemberBaseThru<T, I> &other)
        : mpParent(0),
          mName(other.mName),
          mpArrParent(0)
    {} // NOOP
    
    virtual ~CPH5CompMemberBaseThru() {}
    
    /*!
     * \brief Basic read function that can be called to read this member at
     *        this location in the tree in the target HDF5 file and return a 
     *        reference to the object that was read. Internally this object
     *        is maintained by this object.
     * \return Reference to value that was just read from the arget HDF5 file.
     */
    const T &get() const;
    
    
    /*!
     * \brief Returns the name of this compound member visible in the target
     *        HDF5 file.
     * \return std::string of the name.
     */
    std::string getName() const {
        return mName;
    }
    
    /*!
     * \brief Returns the type of this compound member visible in the target
     *        HDF5 file.
     * \return H5::DataType of type.
     */
    H5::DataType getType() const {
        return mType;
    }
    
    /*!
     * \brief Returns the size of this object as stored in memory.
     * \return int for size.
     */
    int getSize() const {
        return sizeof(T);
    }
    
    /*!
     * \brief Reads data from the buffer passed in by ptr and then moves that
     *        pointer (which is a reference) ahead by the number of bytes that
     *        were read from it. Stores this data into local memory to be
     *        written later. Note that this does NOT write to the target HDF5
     *        file.
     * \param ptr Buffer to read data from and then increment.
     */
    void latchAndMove(char *&ptr) {
        memcpy(&mT, ptr, sizeof(T));
        ptr += sizeof(T);
    }
    
    
    /*!
     * \brief Performs the same action as latchAndMove, but performs an
     *        endian swap. Useful for recursive operations requiring
     *        an endian swap.
     * \param ptr Buffer to read data from and then increment. Buffer is
     *        unchanged, only internal value is endian-swapped.
     */
    void latchAndMoveWithSwap(char *&ptr) {
        memcpy(&mT, ptr, sizeof(T));
        CPH5Swappers::swap_in_place(&mT);
        ptr += sizeof(T);
    }
    
    
    
    /*!
     * \brief Writes data from local memory into the buffer passed in by ptr
     *        and then increments ptr (which is a reference) by the number of
     *        bytes written. Used to read data into a larger chunk of memory 
     *        for the whole compound type member-by-member. Often called
     *        after a raw read into such a chunk of memory. Note that this does
     *        NOT read from the target HDF5 file.
     * \param ptr Buffer to write data into and then increment.
     */
    void copyAndMove(char *&ptr) const {
        memcpy(ptr, &mT, sizeof(T));
        ptr += sizeof(T);
    }
    
    /*!
     * \brief operator = Used to write data from a value into both local memory
     *        and to the target HDF5 file simultaneously. Overloaded operator.
     * \param other Value to write.
     */
    void operator=(const T &other);
    
    
    
    /*!
     * \brief If this member instance is a sub-element (recursively) of an 
     *        compound member array type, it cannot be individually read/written,
     *        because the elements within an array cannot be sub-divided.
     *        This function is called by the highest level array and recursively
     *        executed through all children to notify them that reads and writes
     *        need to be executed at the array level.
     * \param pArrParent Pointer to the inverse-recursive parent array object.
     */
    void setArrayParent(CPH5CompMemberArrayBase *pArrParent) {
        mpArrParent = pArrParent;
    }
    
    
    /*!
     * \brief getStrOfValue Converts the value stored in this element to a string.
     *        Useful for printing contents to a human-readable form.
     * \return String of value.
     */
    std::string getStrOfValue() {
        std::ostringstream outStr;
        // When T is uint8_t, the output string is interpreting this as a char.
        // We don't want it to do this.
        if (sizeof(T) == 1) {
            outStr << (int)get();
        } else {
            outStr << get();
        }
        return outStr.str();
    }
    
    //TODO document
    virtual CPH5LeafType getLeafType() const override {
        return static_cast<CPH5LeafType>(CPH5TreeNode::IsLeaf<T>::Get);
    }
    
    //TODO document
    virtual bool getValIfLeaf(void *p) override {
        *reinterpret_cast<T*>(p) = get();
        return true;
    }
    
    //TODO document
    virtual bool canIndexInto() const override {
        return false;
    }
    
    //TODO document
    virtual CPH5TreeNode *indexInto(int i) override {
        return 0;
    }
    
    //TODO document
    virtual int getIndexableSize() const override {
        return 0;
    }
    
    //TODO document
    virtual CPH5LeafType getElementType() const override {
        // Not an array, therefore has no element type.
        return LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual int getMemorySizeBelow() const override {
        return sizeof(T);
    }
    
    //TODO document
    virtual bool readAllBelow(void *p) override {
        *reinterpret_cast<T*>(p) = get();
        return true;
    }
    
    //TODO document
    void *getMemoryLocation() const override {
        return &mT;
    }
    
    //TODO document
    virtual std::vector<std::string> getChildrenNames() const override {
        return std::vector<std::string>();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string name) const override {
        return 0;
    }
    
    
protected:
    CPH5CompType *mpParent;
    std::string mName;
    H5::DataType mType;
    mutable T mT;
    CPH5CompMemberArrayBase *mpArrParent;
};


/*!
 * \brief The CPH5CompMemberBaseThru<T, _Tp2> class is similar to the other non
 *        specialized version of CPH5CompMemberBaseThru except this
 *        implementation is specialized for if the type of the compound member
 *        is itself a compound type. This class inherits from T to make T's
 *        members available via dot (.) accessing. As such, this object is both
 *        a compound member <i>and</i> a compound type.
 */
template<class T>
class CPH5CompMemberBaseThru<T, IS_DERIVED>
        : public T, public CPH5CompMemberBase
{
    // INHERITED SPECIALIZATION from CPH5CompType
public:
    
    
    /*!
     * \brief CPH5CompMemberBaseThru Constructor. Since this is for a compound
     *        type member we do not accept the H5::DataType to use and instead
     *        derive it from the template parameter T. 
     * \param parent Compound type object to which this member belongs.
     * \param name Name to assign to member visible in the target HDF5 file.
     */
    CPH5CompMemberBaseThru(CPH5CompType *parent,
                           std::string name)
        : mpParent(parent),
          mName(name)
    {
        mType = T().getCompType();
    }
    
    /*!
     * \brief CPH5CompMemberBaseThru Constructor. This variant of the
     *        constructor is specific for the case where the user wishes
     *        to specify a type different from the type derived from the
     *        template parameter. 
     * \param parent Compound type object to which this member belongs.
     * \param name Name to assign to member visible in the target HDF5 file.
     * \param type H5::CompType to use instead of the comptype derived from
     *        the template parameter.
     */
    CPH5CompMemberBaseThru(CPH5CompType *parent,
                           std::string name,
                           H5::CompType type)
        : mpParent(parent),
          mName(name),
          mType(type)
    {} // NOOP
    
    
    /*!
     * \brief Empty constructor for if a standalone compound member object is
     *        desired.
     */
    CPH5CompMemberBaseThru()
        : mpParent(0)
    {} // NOOP standalone object
    
    virtual ~CPH5CompMemberBaseThru() {}
    
    /*!
     * \brief Returns the pointer to the CPH5IOFacility used in this dataset
     *        (or attribute) tree. Must be implemented since this class
     *        inherits from the template parameter which is a CPH5CompType,
     *        and CPH5CompType expects this to exist.
     * \return Pointer to CPH5IOFacility object, or 0 if error.
     */
    CPH5IOFacility *getIOFacility() const;
    
    
    /*!
     * \brief Returns the name of this compound member visible in the target
     *        HDF5 file.
     * \return std::string of the name.
     */
    std::string getName() const {
        return mName;
    }
    
    
    /*!
     * \brief Returns the type of this compound member visible in the target
     *        HDF5 file. For this specialization of CPH5CompMemberBaseThru it
     *        will be an H5::CompType.
     * \return H5::DataType of type.
     */
    H5::DataType getType() const {
        return mType;
    }
    
    
    /*!
     * \brief Returns the size of this object as stored in memory - calculated
     *        by adding up the sum of all children members since this is a 
     *        compound type object.
     * \return int for size.
     */
    int getSize() const {
        return T::getTotalMemorySize();
    }
    
    
    /*!
     * \brief Reads data from the buffer passed in by ptr and then moves that
     *        pointer (which is a reference) ahead by the number of bytes that
     *        were read from it. This is done recursively for all children
     *        CompMember objects since this is a compound type. Note that this
     *        does NOT write to the target HDF5 file.
     * \param ptr Buffer to read data from and then increment.
     */
    void latchAndMove(char *&ptr) {
        T::latchAllAndMove(ptr);
    }
    
    
    
    
    /*!
     * \brief Performs the same action as latchAndMove, but performs an
     *        endian swap. Useful for recursive operations requiring
     *        an endian swap.
     * \param ptr Buffer to read data from and then increment. Buffer is
     *        unchanged, only internal value is endian-swapped.
     */
    void latchAndMoveWithSwap(char *&ptr) {
        T::latchAllAndMoveWithSwap(ptr);
    }

    
    
    
    
    /*!
     * \brief Writes data from local memory into the buffer passed in by ptr
     *        and then increments ptr (which is a reference) by the number of
     *        bytes written. Used to read data into a larger chunk of memory 
     *        for the whole compound type member-by-member. Often called
     *        after a raw read into such a chunk of memory. This is done
     *        recursively for all children CompMember objects since this is
     *        a compound type. Note that this does NOT read from the target
     *        HDF5 file.
     * \param ptr Buffer to write data into and then increment.
     */
    void copyAndMove(char *&ptr) const {
        T::copyAllAndMove(ptr);
    }
    
    
    /*!
     * \brief Special Inverse-Recursive (IR) function that is necessary to 
     *        facilitate writing of members of nested compound types
     *        individually. 
     * \param leaf The H5::CompType generated by the nested compound type
     *        object/member.
     * \return A fully nested H5::CompType object with only the requested
     *         member present.
     */
    H5::CompType nestCompTypeIR(H5::CompType leaf);
    
    
    /*!
     * \brief operator = Overloaded assignment operator that passes assignment
     *        through to the CPH5CompType assignment operator.
     * \param other CPH5CompType object to assign from.
     */
    void operator=(const T &other) {
        T::operator=(other);
    }
    
    
    /*!
     * \brief getStrOfValue Converts the value stored in this element to a string.
     *        Useful for printing contents to a human-readable form.
     * \return String of value.
     */
    std::string getStrOfValue() {
        return std::string();
    }
    
    
    /*!
     * \brief If this member instance is a sub-element (recursively) of an 
     *        compound member array type, it cannot be individually read/written,
     *        because the elements within an array cannot be sub-divided.
     *        This function is called by the highest level array and recursively
     *        executed through all children to notify them that reads and writes
     *        need to be executed at the array level.
     * \param pArrParent Pointer to the inverse-recursive parent array object.
     */
    void setArrayParent(CPH5CompMemberArrayBase *pArrParent);
    
    //TODO document
    virtual CPH5LeafType getLeafType() const override {
        // A compound type is never a leaf;
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual bool getValIfLeaf(void *p) override {
        // A compound type is never a leaf;
        return false;
    }
    
    //TODO document
    virtual bool canIndexInto() const override {
        // This is not an array
        return false;
    }
    
    //TODO document
    virtual CPH5TreeNode *indexInto(int i) override {
        // This is not an array
        return 0;
    }
    
    //TODO document
    virtual int getIndexableSize() const override {
        // This is not an array
        return 0;
    }
    
    //TODO document
    virtual CPH5LeafType getElementType() const override {
        // Not an array, therefore has no element type.
        return LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual int getMemorySizeBelow() const override {
        return getSize();
    }
    
    //TODO document
    virtual bool readAllBelow(void *p) override {
        // This cannot be implemented here, must be implemented
        // in CPH5CompMember
        return false;
    }
    
    //TODO document
    virtual void *getMemoryLocation() const override {
        return 0;
    }
    
    //TODO document
    virtual std::vector<std::string> getChildrenNames() const override {
        return T::getChildrenNames();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string name) const override {
        return dynamic_cast<CPH5TreeNode*>(T::getMemberByName(name));
    }
    
protected:
    CPH5CompType *mpParent;
    std::string mName;
    H5::DataType mType;
};


/*!
 * \brief The CPH5CompType class is a base class for users to override when
 *        creating custom HDF5 compound datatypes. It should be subclassed,
 *        and then have members added to it using the wrapper template classes
 *        CPH5CompMember or CPH5CompMemberArray.
 * 
 * Example: <pre>
 * struct MyType : public CPH5CompType {
 *    CPH5CompMember<int> intMem;
 *    CPH5CompMember<double> dblMem;
 * 
 *    MyType()
 *      : intMem(this, "intMem", H5::PredType::NATIVE_INT),
 *        dblMem(this, "dblMem", H5::PredType::NATIVE_DOUBLE)
 *    {} // NOOP
 * }</pre>
 */
class CPH5CompType
{
public:
    
    /*!
     * \brief CPH5CompType Constructor. Does nothing special.
     */
    CPH5CompType()
        : mpFacility(0),
          mCompTreeWrapper(this),
          mpArrParent(0)
    {} // NOOP
    
    /*!
     * \brief CPH5CompType Destructor. Only deletes external children, if
     *        any exist.
     */
    virtual ~CPH5CompType() {
        // Delete all the external children
        for (int i = 0; i < mExternalChildren.size(); ++i) {
            delete mExternalChildren.at(i);
        }
        mExternalChildren.clear();
    }
    
    /*!
     * \brief Iterates recursively over all children and creates an
     *        H5::CompType representing this compound type.
     * \return The H5::CompType to be visible in the target HDF5 file.
     */
    virtual H5::CompType getCompType()
    {
        // Get the total size of everything
        size_t size = 0;
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            size += (*it)->getSize();
        }
        H5::CompType h5CompType(size);
        size = 0;
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            h5CompType.insertMember((*it)->getName(),
                                    size,
                                    (*it)->getType());
            size += (*it)->getSize();
        }
        return h5CompType;
    }
    
    
    /*!
     * \brief Registers a CPH5CompMember as belonging to this comptype object.
     * \param member CPH5CompMember pointer to register.
     */
    void registerMember(CPH5CompMemberBase *member)
    {
        mChildren.push_back(member);
        //CPH5CompType *pc = dynamic_cast<CPH5CompType*>(member);
        //if (pc != 0) {
        //    pc->setIOFacility(mpFacility);
        //}
    }
    
    /*!
     * \brief Registers a CPH5CompMember as belonging to this comptype object,
     *        but adds it to the list of objects to be deleted upon
     *        destruction.
     * \param member CPH5CompMember pointer to register.
     */
    void registerExternalMember(CPH5CompMemberBase *member)
    {
        mExternalChildren.push_back(member);
        if (mpArrParent != 0) {
            member->setArrayParent(mpArrParent);
        }
    }
    
    /*!
     * \brief Sets the CPH5IOFacility object that this compound object should
     *        use when reading and writing the target HDF5 file. This is called
     *        by the parent dataset or attribute or compound member array object.
     * \param facility Pointer to CPH5IOFacility object to use.
     */
    void setIOFacility(CPH5IOFacility *facility)
    {
        mpFacility = facility;
        //for (int i = 0; i < mChildren.size(); ++i) {
        //    CPH5CompType *pc = dynamic_cast<CPH5CompType*>(mChildren.at(i));
        //    if (pc != 0) {
        //        pc->setIOFacility(mpFacility);
        //    }
        //}
    }
    
    
    /*!
     * \brief Returns the pointer to the currently set CPH5IOFacility to use
     *        when reading or writing the target HDF5 file.
     * \return Pointer to CPH5IOFacility currently stored, even if it is 0.
     */
    virtual CPH5IOFacility *getIOFacility() const
    {
        //if (mpFacility == 0 && mpArrParent != 0) {
        //    mpFacility = mpArrParent->getIoFacility();
        //}
        return mpFacility;
    }
    
    
    
    // Doing this latch here will then allow the default assignment operators
    // for the members to copy a valid value.
    /*!
     * \brief operator = Overloaded assignment operator. Used to read data from
     *        another CPH5CompType object that belongs to a dataset or
     *        attribute, and store into this object.
     * \param other Object to have read data from file and copy data from.
     * \return Reference to this.
     * 
     * Calls the others latchAll function which will read all data from the
     * target HDF5 file into the other object's local memory, if it belongs
     * to an object tree with an open target HDF5 file. This function <b>
     * ONLY</b> tells the other to latch its data and does not explicitly
     * copy the data over. After this function executes, all of the assignment
     * operators for the members get called and the data is copied member-
     * per-member at that point.
     */
    CPH5CompType &operator=(CPH5CompType &other) {
        other.latchAll();
        return *this;
    }
    
    
    
    /*!
     * \brief If this member instance is a sub-element (recursively) of an 
     *        compound member array type, it cannot be individually read/written,
     *        because the elements within an array cannot be sub-divided.
     *        This function is called by the highest level array and recursively
     *        executed through all children to notify them that reads and writes
     *        need to be executed at the array level.
     * \param pArrParent Pointer to the inverse-recursive parent array object.
     */
    // If this is called it means this compound object is a member of an array.
    // All reads and writes need to happen via the parent array.
    void setArrayParent(CPH5CompMemberArrayBase *pArrParent) {
        mpArrParent = pArrParent;
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            (*it)->setArrayParent(pArrParent);
        }
    }
    
    
    
    /*!
     * \brief Calling this will write all data stored in local memory in the
     *        members to the target HDF5 file, if it is open.
     */
    void writeAll()
    {
        H5::CompType type = getCompType();
        size_t size = type.getSize();
        char *buf = new char[size];
        char *ptr = buf;
        
        try {
            if (mpFacility != 0) {
                for(ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it) {
                    (*it)->copyAndMove(ptr);
                }
                mpFacility->write(buf, type);
            }
        } catch (...) {
            delete[] buf;
            throw;
        }
        
        
        delete[] buf;
    }
    
    
    
    /*!
     * \brief Reads all members from the target HDF5 file, if it is open, into
     *        the members local memory.
     */
    void readAll()
    {
        H5::CompType type = getCompType();
        size_t size = type.getSize();
        char *buf = new char[size];
        char *ptr = buf;
        
        try {
            if (mpFacility != 0) {
                mpFacility->read(buf, type);
                for(ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it) {
                    (*it)->latchAndMove(ptr);
                }
            }
        } catch (...) {
            delete[] buf;
            throw;
        }
        
        
        delete[] buf;
    }
    
    
    /*!
     * \brief Iterates over all children and calls their copyAndMove function.
     *        Results in data being copied from the members local storage into
     *        the buffer (and the pointer reference to the buffer being 
     *        incremented).
     * \param ptr Buffer to pass to children to copy data into.
     */
    void copyAllAndMove(char *&ptr) const
    {
        if (!mChildren.empty()) {
            for(ChildList::const_iterator it = mChildren.cbegin();
                it != mChildren.end();
                ++it) {
                (*it)->copyAndMove(ptr);
            }
        }
    }
    
    
    /*!
     * \brief Iterates over all children and calls their latchAndMove function.
     *        Results in data being copied from the buffer given into the
     *        members local storage (and the pointer reference to the buffer 
     *        being incremented).
     * \param ptr Pointer to pass to children to copy data from.
     */
    void latchAllAndMove(char *&ptr)
    {
        if (!mChildren.empty()) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->latchAndMove(ptr);
            }
        }
    }
    
    
    /*!
     * \brief Performs the same action as latchAndMove, but performs an
     *        endian swap. Useful for recursive operations requiring
     *        an endian swap.
     * \param ptr Buffer to read data from and then increment. Buffer is
     *        unchanged, only internal value is endian-swapped.
     */
    void latchAllAndMoveWithSwap(char *&ptr) {
        if (!mChildren.empty()) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->latchAndMoveWithSwap(ptr);
            }
        }
    }
    
    
    
    
    /*!
     * \brief Calculates the total memory size of all the members belonging to
     *        this compound type.
     * \return The total memory size of this compound type.
     */
    int getTotalMemorySize() const {
        int ret = 0;
        if (mChildren.size() > 0) {
            for(ChildList::const_iterator it = mChildren.cbegin();
                it != mChildren.end();
                ++it) {
                ret += (*it)->getSize();
            }
        }
        return ret;
    }
    
    
    /*!
     * \brief This is the terminal nestCompTypeIR function. See CPH5CompMember
     *        nestCompTypeIR function. Does not do anything.
     * \param leaf CompType parameter
     * \return The parameter, without changing it.
     */
    virtual H5::CompType nestCompTypeIR(H5::CompType leaf)
    {
        return leaf;
    }
    
    
    int numChildren() const {
        return mChildren.size();
    }
    
    
    CPH5CompMemberBase *getChildAt(int ind) {
        if (ind >= 0 && ind < mChildren.size()) {
            return mChildren[ind];
        }
		return nullptr;
    }
    
    // CPH5CompType is not and cannot be a true TreeNode subclass.
    // But, it is needed to be a treenode. Make a treenode subclass
    // member class inside this and modify the function in
    // CompMemberArray that returns a reference to an element
    // (for compound arrays) to go 1 step further and retrieve
    // the subclass reference to this non-treenode treenode.
    
    class CompTreeWrapper : public CPH5TreeNode {
    public:
        CompTreeWrapper(CPH5CompType *parent)
            : mParent(parent)
        {} // NOOP
        
        //TODO document
        virtual CPH5LeafType getLeafType() const override {
            // Compound types are never leaves
            return LT_IS_NOT_LEAF;
        }
        
        //TODO document
        virtual bool getValIfLeaf(void *p) override {
            return false;
        }
        
        //TODO document
        virtual bool canIndexInto() const {
            return false;
        }
        
        //TODO document
        virtual CPH5TreeNode *indexInto(int i) override {
            return 0;
        }
        
        //TODO document
        virtual int getIndexableSize() const override {
            return 0;
        }
        
        //TODO document
        CPH5LeafType getElementType() const {
            // A compound type object is never an array
            return LT_IS_NOT_LEAF;
        }
        
        //TODO document
        int getMemorySizeBelow() const {
            if (mParent == 0) {
                return 0;
            }
            return mParent->getTotalMemorySize();
        }
        
        //TODO document
        bool readAllBelow(void *p) {
            if (mParent == 0) {
                return false;
            }
            mParent->readAll();
            char *temp = reinterpret_cast<char*>(p);
            mParent->copyAllAndMove(temp);
            return true;
        }
        
        //TODO document
        void *getMemoryLocation() const override {
            return 0;
        }
        
        //TODO document
        virtual std::vector<std::string> getChildrenNames() const override {
            return mParent->getChildrenNames();
        }
        
        //TODO document
        virtual CPH5TreeNode *getChildByName(std::string name) const override {
            return dynamic_cast<CPH5TreeNode*>(mParent->getMemberByName(name));
        }
        
        
    private:
        CPH5CompType *mParent;
    };
    
    CPH5TreeNode *getTreeNode() const {
        return dynamic_cast<CPH5TreeNode*>(&mCompTreeWrapper);
    }
    
    //TODO document
    std::vector<std::string> getChildrenNames() const {
        std::vector<std::string> ret;
        if (mChildren.empty()) {
            return ret;
        }
        for (int i = 0; i < numChildren(); ++i) {
            ret.push_back(mChildren.at(i)->getName());
        }
        return ret;
    }
    
    //TODO document
    CPH5CompMemberBase *getMemberByName(std::string name) const {
        if (mChildren.empty()) {
            return 0;
        }
        for (int i = 0; i < numChildren(); ++i) {
            if (mChildren.at(i)->getName() == name) {
                return mChildren.at(i);
            }
        }
        return 0;
    }
    
    
protected:
    
    
    /*!
     * \brief Iterates over all children and reads the entire comptype from
     *        the target HDF5 file.
     */
    void latchAll()
    {
        H5::CompType type = getCompType();
        size_t size = type.getSize();
        char *buf = new char[size];
        char *ptr = buf;
        
        try {
            if (mpFacility != 0) {
                mpFacility->read(buf, type);
                for(ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it) {
                    (*it)->latchAndMove(ptr);
                }
            }
        } catch (...) {
            delete[] buf;
            throw;
        }
        
        delete[] buf;
    }
    
    typedef std::vector<CPH5CompMemberBase *> ChildList;
    ChildList mChildren;
    ChildList mExternalChildren;
    
    mutable CPH5IOFacility *mpFacility;
    
    CPH5CompMemberArrayBase *mpArrParent;
    
private:
    
    CPH5CompType(CPH5CompType &&other); // Disabled move
    CPH5CompType &operator=(CPH5CompType &&other); // Disabled move-assign
    
    mutable CompTreeWrapper mCompTreeWrapper;
};


/*!
 * \brief The CPH5CompMember class is a generic template 
 *        container for members of a compound datatype inside
 *        of a CPH5CompType-derived class.
 * 
 * The CPH5CompMember class is templated with the type that it
 * should be treated as in code, and constructed with the H5
 * type that it should use to store into the file, unless it is
 * also a compound type, in which case the type will be derived
 * from the template parameter.
 * 
 * This should belong to a subclass of a CPH5CompType object and
 * initialized in that objects constructor list.
 * 
 * Additionally, this class inherits from CPH5CompMemberBaseThru,
 * which is specialized for compound and non-compound types in order
 * to change the functionality as necessary.
 */
template<class T>
class CPH5CompMember
        : public CPH5CompMemberBaseThru<T, IsDerivedFrom<T, CPH5CompType>::Is>
{
    typedef CPH5CompMemberBaseThru<T, IsDerivedFrom<T, CPH5CompType>::Is> CPH5CompMemberBaseThruSpec;
public:
    
    /*!
     * \brief CPH5CompMember Constructor that should be used when creating a
     *        member with a non-compound type. Attempting to use this
     *        constructor with a compound type will result in an error.
     *        This constructor registers itself with parent if parent is not 0.
     * \param parent CPH5CompType subclass to add this member to.
     * \param name Name of this compound member visible in the target HDF5
     *        file.
     * \param type H5::DataType to assign to this compound member in the target
     *        HDF5 file.
     */
    CPH5CompMember(CPH5CompType *parent,
                   std::string name,
                   H5::DataType type)
        : CPH5CompMemberBaseThruSpec(parent, name, type)
    {
        // Part of a structure
        if (CPH5CompMemberBaseThruSpec::mpParent != 0)
            CPH5CompMemberBaseThruSpec::mpParent->registerMember(this);
    }
    
    /*!
     * \brief CPH5CompMember Constructor that should be used when creating a
     *        member with a compound type, but where the user wishes to 
     *        supply an H5 type parameter instead of having it derived from
     *        the template parameter.
     * \param parent CPH5CompType subclass to add this member to.
     * \param name Name of this compound member visible in the target HDF5
     *        file.
     * \param type H5::CompType to assign to this compound member in the target
     *        HDF5 file.
     */
    CPH5CompMember(CPH5CompType *parent,
                   std::string name,
                   H5::CompType type)
        : CPH5CompMemberBaseThruSpec(parent, name, type)
    {
        // Part of a structure
        if (CPH5CompMemberBaseThruSpec::mpParent != 0)
            CPH5CompMemberBaseThruSpec::mpParent->registerMember(this);
    }
    
    /*!
     * \brief CPH5CompMember Constructor that should be used when creating a
     *        member <i>with</i> a compound type. Attempting to use this
     *        constructor with a non-compound template type will result in an
     *        error. This constructor registers itself with parent if parent is
     *        not 0.
     * \param parent CPH5CompType subclass to add this member to.
     * \param name Name of this compound member visible in the target HDF5
     *        file.
     */
    CPH5CompMember(CPH5CompType *parent,
                   std::string name)
        : CPH5CompMemberBaseThruSpec(parent, name)
    {
        // Part of a structure
        if (CPH5CompMemberBaseThruSpec::mpParent != 0)
            CPH5CompMemberBaseThruSpec::mpParent->registerMember(this);
    }
    
    
    /*!
     * \brief CPH5CompMember Constructor to be used when creating a standalone
     *        member object. Takes no parameters and does not attempt to
     *        register with parent.
     */
    CPH5CompMember()
        : CPH5CompMemberBaseThruSpec()
    {} // NOOP
    
    virtual ~CPH5CompMember() {}
    
    /*!
     * \brief operator T Used to read value from target HDF5 file and return
     *        for storage into a T object.
     * 
     * NOTE Using this operator will result in a compiler error if T is a
     * compound type. Future fix.
     */
    operator T() const {
        return CPH5CompMemberBaseThruSpec::get();
    }
    
    
    /*!
     * \brief operator = Overloaded assignment operator for writing to compound
     *        member in target HDF5 file.
     * \param rhs Value to write to target HDF5 file.
     */
    void operator=(const T &rhs) {
        CPH5CompMemberBaseThruSpec::operator=(rhs);
    }
    
    
    /*!
     * \brief operator = Overloaded copy assignment constructor for copying
     *        from another CPH5CompMember. Copies the name, datatype, and
     *        local storage of other member but not parent.
     * \param other CPH5CompMember to copy from.
     * \return 
     */
    CPH5CompMember<T> &operator=(const CPH5CompMember<T> &other) {
        //CPH5CompMemberBaseThruSpec::mpParent = 0;
        CPH5CompMemberBaseThruSpec::mT = other.mT;
        CPH5CompMemberBaseThruSpec::mName = other.mName;
        CPH5CompMemberBaseThruSpec::mType = other.mType;
        return *this;
    }
    
    
    /*!
     * \brief CPH5CompMember Copy constructor. Exactly the same as the
     *        overloaded copy assignment constructor.
     * \param other CPH5CompMember to copy from.
     */
    CPH5CompMember(const CPH5CompMember<T> &other) {
        //CPH5CompMemberBaseThruSpec::mpParent = 0;
        CPH5CompMemberBaseThruSpec::mT = other.mT;
        CPH5CompMemberBaseThruSpec::mName = other.mName;
        CPH5CompMemberBaseThruSpec::mType = other.mType;
    }
    
    // TreeNode functions not required here because they are present in all
    // specializations of CPH5CompMemberBaseThru
    
};







/*!
 * \brief operator << Global overload for the << (streaming) operator for
 *        convenience if output to std::cout is used.
 * \param lhs Stream to output to.
 * \param rhs CPH5CompMember object to output value to.
 * \return Reference to lhs;
 */
template<class T1, class T2>
T1 &operator<<(T1 &lhs, CPH5CompMember<T2> &rhs) {
    lhs << (T2)rhs;
    return lhs;
}


/*!
 * \brief The CPH5CompMemberArrayBaseThru class is a pass-through, templated
 *        version of CPH5CompMemberArrayBase to use for testing the type
 *        of an array with dynamic_cast.
 */
template<class T>
class CPH5CompMemberArrayBaseThru : public CPH5CompMemberArrayBase
{
public:
    virtual ~CPH5CompMemberArrayBaseThru() {}
    virtual std::string getName() const = 0;
    virtual H5::DataType getType() const = 0;
    virtual int getSize() const = 0;
    virtual int getNumElements() const = 0;
    virtual void latchAndMove(char *&ptr) = 0;
    virtual void latchAndMoveWithSwap(char *&ptr) = 0;
    virtual void copyAndMove(char *&ptr) const = 0;
    virtual std::string getStrOfValue() = 0;
    virtual void setArrayParent(CPH5CompMemberArrayBase *pArrParent) = 0;
    virtual CPH5IOFacility *getIoFacility() = 0;
};


/*!
 * \brief The CPH5CompMemberArrayBaseInherited class is a pass-through version
 *        of the CPH5CompMemberArrayBase to use for testing whether an array
 *        is an array of compound types using dynamic_cast.
 */
class CPH5CompMemberArrayBaseInherited : public CPH5CompMemberArrayBase
{
public:
    virtual ~CPH5CompMemberArrayBaseInherited() {}
    virtual std::string getName() const = 0;
    virtual H5::DataType getType() const = 0;
    virtual int getSize() const = 0;
    virtual int getNumElements() const = 0;
    virtual void latchAndMove(char *&ptr) = 0;
    virtual void latchAndMoveWithSwap(char *&ptr) = 0;
    virtual void copyAndMove(char *&ptr) const = 0;
    virtual std::string getStrOfValue() = 0;
    virtual CPH5CompType *getCompTypeObjAt(int index) = 0;
    virtual void setArrayParent(CPH5CompMemberArrayBase *pArrParent) = 0;
    virtual CPH5IOFacility *getIoFacility() = 0;
};


/*!
 * \brief The CPH5CompMemberArrayBaseThruInherited class is a pass-through, templated
 *        version of the CPH5CompMemberArrayBaseInherited class to use for testing
 *        for the member type of the array using dynamic_cast.
 */
template<class T>
class CPH5CompMemberArrayBaseThruInherited : public CPH5CompMemberArrayBaseInherited
{
public:
    virtual ~CPH5CompMemberArrayBaseThruInherited() {}
    virtual std::string getName() const = 0;
    virtual H5::DataType getType() const = 0;
    virtual int getSize() const = 0;
    virtual int getNumElements() const = 0;
    virtual void latchAndMove(char *&ptr) = 0;
    virtual void latchAndMoveWithSwap(char *&ptr) = 0;
    virtual void copyAndMove(char *&ptr) const = 0;
    virtual std::string getStrOfValue() = 0;
    virtual CPH5CompType *getCompTypeObjAt(int index) = 0;
    virtual void setArrayParent(CPH5CompMemberArrayBase *pArrParent) = 0;
    virtual CPH5IOFacility *getIoFacility() = 0;
};



/*!
 * \brief The CPH5CompMemberArrayCommon class is the base class containing all
 *        of the functionality for an array of items as a compound member.
 * 
 * Currently, only single dimension arrays of elements are supported. Arrays of
 * compound types are supported.
 * 
 * Note that the second parameter is different from the CPH5Dataset class in
 * that it is the number of elements in the array instead of the number of
 * dimensions. By extension, CompMemberArrays are only supported with a fixed
 * size currently.
 * 
 * Additionally, the constructor for this class is protected in order to ensure
 * that this class cannot be implemented directly - it is made public in
 * CPH5CompMemberArray which inherits this class.
 */
template<class T, const int inh>
class CPH5CompMemberArrayCommon : public CPH5CompMemberArrayBaseThru<T>
{
    // NON-INHERITED SPECIALIZATION
    
public:
    
    /*!
     * \brief The ElementProxy class is a standard proxy implementation
     *        pointing to a single element in a CompMemberArray, allowing the
     *        user to access an element via assignment and typecasting.
     * 
     *        Should only be used with non-compound types.
     */
    template<class E>
    class ElementProxy : public CPH5TreeNode
    {
    public:
        /*!
         * \brief Constructor, noop other than initializing members.
         * \param p CompMemberArray that holds data.
         * \param index Index that this element is in reference to.
         */
        ElementProxy(CPH5CompMemberArrayCommon<E, inh> *p,
                    int index)
            : mP(p),
              ind(index)
        {} // NOOP
        
        ElementProxy()
            : mP(0),
              ind(-1)
        {} // NOOP
        
        /*!
         * \brief operator = Overloaded assignment operator. Calls the write
         *        function of the CompMemberArray with the given index.
         * \param other Value to write.
         */
        void operator=(const E other) {
            mP->write(other, ind);
        }
        
        /*!
         * \brief operator E Overloaded typecast operator. Calls the read
         *        function of the CompMemberArray with the given index, and
         *        returns the value read.
         */
        operator E() {
            return mP->read(ind);
        }
        
        //TODO document
        virtual CPH5LeafType getLeafType() const override {
            return static_cast<CPH5LeafType>(IsLeaf<E>::Get);
        }
        
        //TODO document
        virtual bool getValIfLeaf(void *p) override {
            *reinterpret_cast<E*>(p) = mP->read(ind);
            return true;
        }
        
        //TODO document
        virtual bool canIndexInto() const override {
            return false;
        }
        
        //TODO document
        virtual CPH5TreeNode *indexInto(int i) override {
            return 0;
        }
        
        //TODO document
        virtual int getIndexableSize() const override {
            return 0;
        }
        
        //TODO document
        virtual CPH5LeafType getElementType() const override {
            // This is an element of an array, not an array.
            return LT_IS_NOT_LEAF;
        }
        
        //TODO document
        virtual int getMemorySizeBelow() const override {
            return sizeof(E);
        }
        
        //TODO document
        virtual bool readAllBelow(void *p) override {
            mP->read((E*)p);
            return true;
        }
        
        //TODO document
        virtual void *getMemoryLocation() const override {
            return mP->getMemoryLocation();
        }
        
        //TODO document
        virtual std::vector<std::string> getChildrenNames() const override {
            return std::vector<std::string>();
        }
        
        //TODO document
        virtual CPH5TreeNode *getChildByName(std::string name) const override {
            return 0;
        }
        
        
    private:
        CPH5CompMemberArrayCommon<E, inh> *mP;
        int ind;
    };
    
    
    /*!
     * \brief Constructor similar to the basic CPH5CompMember constructor,
     *        except that there is no option to exclude the H5::DataType to use
     *        since this class is a non-inherited specialization.
     * \param parent The CompType subclass to which this member belongs.
     * \param name The name to assign to this member visible in the target
     *        HDF5 file.
     * \param type The datatype to assign to this member visible in the targer
     *        HDF5 file.
     */
    CPH5CompMemberArrayCommon(CPH5CompType *parent,
                              std::string name,
                              H5::DataType type,
                              int nElements)
        : mpParent(parent),
          mName(name),
          mReadDone(false),
          mpArrParent(0)
    {
        // Part of a structure
        if (mpParent != 0)
            mpParent->registerMember(this);
        
        mNElements = nElements;
        
        mBaseType = type;
        hsize_t d[] = {static_cast<hsize_t>(nElements)};
        mpArrType = std::make_shared<H5::ArrayType>(mBaseType, 1, d);
        
        pMT = new T[mNElements];
        memset(pMT, 0, sizeof(T)*nElements);
    }
    
    
    /*!
     * \brief Destructor. Does nothing special.
     */
    virtual ~CPH5CompMemberArrayCommon() {
        mpArrType.get()->close();
        delete[] pMT;
    }
    
public:
    
    
    /*!
     * \brief Returns the name of this compound member visible in the target
     *        HDF5 file.
     * \return std::string of the name.
     */
    std::string getName() const {
        return mName;
    }
    
    /*!
     * \brief Returns the fundamental type (non-array) of this compound member
     *        visible in the target HDF5 file.
     * \return H5::DataType of fundamental type.
     */
    H5::DataType getBaseType() const {
        return mBaseType;
    }
    
    /*!
     * \brief Returns the array type of this compound member visible in the
     *        target HDF5 file. This differs from the other implementations of
     *        getType slightly, however the base type is baked into the array
     *        type so the result is an array with the base type.
     * \return H5::DataType of fundamental type.
     */
    H5::DataType getType() const {
        return *mpArrType.get();
    }
    
    
    /*!
     * \brief Returns the size of this object as stored in memory - calculated
     *        by multiplying the size of the memory type and the number of
     *        elements stored in the array.
     * \return int for size.
     */
    int getSize() const {
        return sizeof(T)*mNElements;
    }
    
    
    /*!
     * \brief getNumElements Returns the number of elements in the array.
     * \return Number of elements in the array.
     */
    int getNumElements() const {
        return mNElements;
    }
    
    
    
    /*!
     * \brief Reads data from the buffer passed in by ptr and then moves that
     *        pointer (which is a reference) ahead by the number of bytes that
     *        were read from it. This is done for the entire array, thus the
     *        pointer will be incremented by the number returned by getSize().
     * \param ptr Buffer to read data from and then increment.
     */
    void latchAndMove(char *&ptr) {
        mReadDone = true;
        memcpy(pMT, ptr, sizeof(T)*mNElements);
        ptr += sizeof(T)*mNElements;
    }
    
    
    
    /*!
     * \brief Performs the same action as latchAndMove, but performs an
     *        endian swap. Useful for recursive operations requiring
     *        an endian swap on each element in the array.
     * \param ptr Buffer to read data from and then increment. Buffer is
     *        unchanged, only internal values are endian-swapped.
     */
    void latchAndMoveWithSwap(char *&ptr) {
        latchAndMove(ptr);
        for (int i = 0; i < mNElements; ++i) {
            CPH5Swappers::swap_in_place(&pMT[i]);
        }
    }
    
    
    
    
    /*!
     * \brief Writes data from local memory into the buffer passed in by ptr
     *        and then increments ptr (which is a reference) by the number of
     *        bytes written. Used to read data into a larger chunk of memory 
     *        for the whole compound type member-by-member. Often called
     *        after a raw read into such a chunk of memory. This is done
     *        for the entire array, thus the pointer will be incremented by the
     *        number returned by getSize();
     * \param ptr Buffer to write data into and then increment.
     */
    void copyAndMove(char *&ptr) const {
        if (!mReadDone) {
            read(pMT);
        }
        memcpy(ptr, pMT, sizeof(T)*mNElements);
        ptr += sizeof(T)*mNElements;
    }
    
    
    /*!
     * \brief Special overload of the write function that allows for individual
     *        element access of the array. Calls the other write function with
     *        the data.
     * \param val Value to write.
     * \param index Index to write value to.
     */
    void write(T val, int index) {
        if (!mReadDone) {
            read(pMT);
        }
        pMT[index] = val;
        write(pMT);
    }
    
    
    /*!
     * \brief Special overload of the read function that allows for individual
     *        element access of the array. Calls the other read function.
     * \param index Index of element to read.
     * \return Value read from target HDF5 file.
     */
    T read(int index) {
        if (!mReadDone) {
            read(pMT);
        }
        return pMT[index];
    }
    
    
    /*!
     * \brief Writes the whole array with the data in buf. Buf must be large
     *        enough to source the entire memory size of the array member from.
     * \param buf Pointer to buffer to source data from
     */
    void write(const T *buf) {
        memcpy(pMT, buf, sizeof(T)*mNElements);
        if (mpParent != 0) {
            CPH5IOFacility *pIO = mpParent->getIOFacility();
            if (pIO != 0) {
                H5::CompType h5CompType(mpArrType.get()->getSize());
                h5CompType.insertMember(mName, 0, *mpArrType.get());
                //pIO->write(mT, h5CompType);
                pIO->write(pMT, mpParent->nestCompTypeIR(h5CompType));
            } else if (mpArrParent != 0) {
                mpArrParent->signalChange();
            }
        }
    }
    
    /*!
     * \brief Reads the whole array from the target HDF5 file into buf. Buf
     *        must be large enough to hold the entire memory size of the array
     *        data.
     * \param buf Pointer to buffer to write data into.
     */
    void read(T *buf) const {
        if (mpParent != 0) {
            CPH5IOFacility *pIO = mpParent->getIOFacility();
            if (pIO != 0) {
                H5::CompType h5CompType(mpArrType.get()->getSize());
                h5CompType.insertMember(mName, 0, *mpArrType.get());
                //pIO->read(mT, h5CompType);
                pIO->read(pMT, mpParent->nestCompTypeIR(h5CompType));
                mReadDone = true;
            }
        }
        memcpy(buf, pMT, sizeof(T)*mNElements);
    }
    
    
    /*!
     * \brief operator [] Overloaded element-access operator that creates and
     *        returns an ElementWard that references this array object and the
     *        given index.
     * \param ind Index of element to access.
     * \return ElementProxy object that references this and the given index, and
     *         allows for both reading via casting and writing via assignment.
     */
    ElementProxy<T> operator[](int ind) {
        return ElementProxy<T>(this, ind);
    }
    
    
    /*!
     * \brief getStrOfValue Converts the value stored in this element to a string.
     *        Useful for printing contents to a human-readable form.
     * \return String of value.
     */
    virtual std::string getStrOfValue() {
        std::vector<T> bufT;
        bufT.resize(mNElements);
        read(bufT.data()); // Values now in pMT;
        std::ostringstream ret;
        for (int i = 0; i < mNElements; ++i) {
            ret << pMT[i] << " ";
        }
        return ret.str();
    }
    
    
    
    /*!
     * \brief If this member instance is a sub-element (recursively) of an 
     *        compound member array type, it cannot be individually read/written,
     *        because the elements within an array cannot be sub-divided.
     *        This function is called by the highest level array and recursively
     *        executed through all children to notify them that reads and writes
     *        need to be executed at the array level.
     * \param pArrParent Pointer to the inverse-recursive parent array object.
     */
    void setArrayParent(CPH5CompMemberArrayBase *pArrParent) {
        mpArrParent = pArrParent;
    }
    
    void signalChange() {
        if (mpArrParent == 0) {
            //Future proper error, for now do nothing
            return;
        }
        mpArrParent->signalChange();
    }
    
    
    //TODO document
    CPH5IOFacility *getIoFacility() {
        if (mpParent != 0) {
            if (mpParent->getIOFacility() != 0) {
                return mpParent->getIOFacility();
            }
        }
        if (mpArrParent != 0) {
            return mpParent->getIOFacility();
        }
        return 0;
    }
    
    
    //TODO document
    virtual CPH5TreeNode::CPH5LeafType getLeafType() const override {
        // Assume the array size is not 0 because that would be stupid.
        // Arrays are never leaves.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual bool getValIfLeaf(void *p) override {
        // Arrays are never leaves.
        return false;
    }
    
    //TODO document
    virtual bool canIndexInto() const override {
        // Assume the array size is not 0 because that would be stupid.
        return true;
    }
    
    //TODO document
    virtual CPH5TreeNode *indexInto(int i) override {
        // Assume the array size is not 0 because that would be stupid.
        mpParent->readAll(); //TODO does this cause unnecessary reads?
        mElemProxy = operator[](i);
        return dynamic_cast<CPH5TreeNode*>(&mElemProxy);
    }
    
    //TODO document
    virtual int getIndexableSize() const override {
        return mNElements;
    }
    
    //TODO document
    virtual CPH5TreeNode::CPH5LeafType getElementType() const override {
        return static_cast<CPH5TreeNode::CPH5LeafType>(CPH5TreeNode::IsLeaf<T>::Get);
    }
    
    //TODO document
    virtual int getMemorySizeBelow() const override {
        return sizeof(T)*mNElements;
    }
    
    //TODO document
    virtual bool readAllBelow(void *p) override {
        read((T*)p);
        return true;
    }
    
    //TODO document
    void *getMemoryLocation() const override {
        return pMT;
    }
    
    //TODO document
    virtual std::vector<std::string> getChildrenNames() const override {
        return std::vector<std::string>();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string name) const override {
        return 0;
    }
    
    
    
protected:
    
    
    
    
    
    
    CPH5CompType *mpParent;
    std::string mName;
    H5::DataType mBaseType;
    //H5::ArrayType mArrType;
	std::shared_ptr<H5::ArrayType> mpArrType;
    int mNElements;
    //mutable T mT[nElements];
    mutable T *pMT;
    mutable bool mReadDone;
    CPH5CompMemberArrayBase *mpArrParent;
    ElementProxy<T> mElemProxy;
};






/*!
 * \brief A specialization of the CPH5CompMemberArrayCommon implementation
 *        for an array of compound types.
 */
template<class T>
class CPH5CompMemberArrayCommon<T, IS_DERIVED> : public CPH5CompMemberArrayBaseThruInherited<T>
{
    // INHERITED SPECIALIZATION from CPH5CompType
    
public:
    
    /*!
     * \brief CPH5CompMemberArrayCommon Constructor.
     * \param parent
     * \param name
     */
    CPH5CompMemberArrayCommon(CPH5CompType *parent,
                              std::string name,
                              int nElements)
        : mpParent(parent),
          mName(name),
          mReadDone(false),
          mpArrParent(0)
    {
        // Part of a structure
        if (mpParent != 0)
            mpParent->registerMember(this);
        
        mBaseType = T().getCompType();
        hsize_t d[] = {nElements};
        mpArrType = std::make_shared<H5::ArrayType>(mBaseType, 1, d);
		
        mNElements = nElements;
        
        pmT = new T[nElements+1];
        // Give all the children a pointer to this as the parent array.
        // If there is a higher array, it will call setArrayParent and
        // we will pass the new pointer down then.
        for (int i = 0; i < nElements; ++i) {
            pmT[i].setArrayParent(this);
        }
    }
    
    
    /*!
     * \brief CPH5CompMemberArrayCommon Constructor for overriding the type
     *        derived from the template argument.
     * \param parent
     * \param name
     */
    CPH5CompMemberArrayCommon(CPH5CompType *parent,
                              std::string name,
                              H5::DataType type,
                              int nElements)
        : mpParent(parent),
          mName(name),
          mReadDone(false),
          mpArrParent(0)
    {
        // Part of a structure
        if (mpParent != 0)
            mpParent->registerMember(this);
        
        mBaseType = type;
        hsize_t d[] = {static_cast<hsize_t>(nElements)};
        mpArrType = std::make_shared<H5::ArrayType>(mBaseType, 1, d);
		
        mNElements = nElements;
        
        pmT = new T[nElements+1];
        // Give all the children a pointer to this as the parent array.
        // If there is a higher array, it will call setArrayParent and
        // we will pass the new pointer down then.
        for (int i = 0; i < nElements; ++i) {
            pmT[i].setArrayParent(this);
        }
    }
    
    
    /*!
     * \brief CPH5CompMemberArrayCommon Destructor.
     */
    virtual ~CPH5CompMemberArrayCommon() {
        delete[] pmT;
    }
    
    
public:
    
    /*!
     * \brief operator [] Element accessor to the array. ElementProxy is
     *        not needed because arrays of compound types are dynamically
     *        allocated and held in memory.
     * \param index Index within array. Must be within bounds
     * \return Reference to element in local memory. 
     */
    T &operator[](int index) {
        // What we are doing here is checking to make sure that if we are the top-level 
        // compmemberarray, check and see if the indices from the parent dataset have changed.
        // If they have changed, we need to re-read all the data into local memory with the new
        // indices.
        CPH5IOFacility *pioFacility = 0;
        if (mpParent != 0) {
            pioFacility = mpParent->getIOFacility();
        }
        if (!mReadDone && pioFacility != 0) {
            readAll();
            mDatasetIndices = pioFacility->getIndices();
        } else if (mpParent != 0 && pioFacility != 0 && mpArrParent == 0) {
            std::vector<int> curIndices = pioFacility->getIndices();
            if (curIndices.size() != mDatasetIndices.size() ||
                !std::equal(curIndices.begin(), curIndices.end(), mDatasetIndices.begin())) {
                readAll();
            }
            mDatasetIndices = curIndices;
        }
        
        if (index >= 0 && index < mNElements) {
            return pmT[index];
        } else {
            // Future: proper error.
            // For now just return placeholder object.
            return pmT[mNElements];
        }
    }
    
    /*!
     * \brief getName Getter for the name of this member.
     * \return Name of this member. Empty if not set.
     */
    std::string getName() const {
        return mName;
    }
    
    /*!
     * \brief Returns the fundamental type (non-array) of this compound member
     *        visible in the target HDF5 file.
     * \return H5::DataType of fundamental type.
     */
    H5::DataType getBaseType() const {
        return mBaseType;
    }
    
    /*!
     * \brief getType Getter for this array members H5 type.
     * \return The H5::DataType for the array, actually an H5::ArrayType.
     */
    H5::DataType getType() const {
        return *mpArrType.get();
    }
    
    /*!
     * \brief getSize Getter for the in-memory size of the elements contained
     *        by this array.
     * \return Memory size of elements in array.
     */
    int getSize() const {
        return mNElements*mBaseType.getSize();
    }
    
    /*!
     * \brief getNumElements Returns the number of elements in the array.
     * \return Number of elements in the array.
     */
    int getNumElements() const {
        return mNElements;
    }
    
    /*!
     * \brief Reads data from the buffer passed in by ptr and then moves that
     *        pointer (which is a reference) ahead by the number of bytes that
     *        were read from it. This is done for the entire array, thus the
     *        pointer will be incremented by the number returned by getSize().
     * \param ptr Buffer to read data from and then increment.
     */
    void latchAndMove(char *&ptr) {
        for (int i = 0; i < mNElements; ++i) {
            pmT[i].latchAllAndMove(ptr);
        }
        mReadDone = true;
    }
    
    
    /*!
     * \brief Performs the same action as latchAndMove, but performs an
     *        endian swap. Useful for recursive operations requiring
     *        an endian swap on each element in the array.
     * \param ptr Buffer to read data from and then increment. Buffer is
     *        unchanged, only internal values are endian-swapped.
     */
    void latchAndMoveWithSwap(char *&ptr) {
        for (int i = 0; i < mNElements; ++i) {
            pmT[i].latchAllAndMoveWithSwap(ptr);
        }
        mReadDone = true;
    }
    
    
    /*!
     * \brief Writes data from local memory into the buffer passed in by ptr
     *        and then increments ptr (which is a reference) by the number of
     *        bytes written. Used to read data into a larger chunk of memory 
     *        for the whole compound type member-by-member. Often called
     *        after a raw read into such a chunk of memory. This is done
     *        for the entire array, thus the pointer will be incremented by the
     *        number returned by getSize();
     * \param ptr Buffer to write data into and then increment.
     */
    void copyAndMove(char *&ptr) const {
        for (int i = 0; i < mNElements; ++i) {
            pmT[i].copyAllAndMove(ptr);
        }
    }
    
    
    /*!
     * \brief getStrOfValue Not implemented for arrays of compound types.
     * \return Empty string.
     */
    std::string getStrOfValue() {
        return ""; // Future enhancement, not implemented for arrays of
                   // compound types.
    }
    
    
    /*!
     * \brief getCompTypeObjAt Convenience accessor for array element access,
     *        relies on operator [].
     * \param index Index within array, must be in bounds.
     * \return Pointer to element.
     */
    CPH5CompType *getCompTypeObjAt(int index) {
        if (index >= 0 && index < mNElements) {
            return &(this->operator [](index));
        } else {
            return 0;
        }
    }
    
    CPH5CompType *getCompTypeObjAtBypass(int index) {
        if (index >= 0 && index < mNElements) {
            return &(pmT[index]);
        } else {
            return 0;
        }
    }
    
    /*!
     * \brief If this member instance is a sub-element (recursively) of an 
     *        compound member array type, it cannot be individually read/written,
     *        because the elements within an array cannot be sub-divided.
     *        This function is called by the highest level array and recursively
     *        executed through all children to notify them that reads and writes
     *        need to be executed at the array level.
     * \param pArrParent Pointer to the inverse-recursive parent array object.
     */
    void setArrayParent(CPH5CompMemberArrayBase *pArrParent) {
        mpArrParent = pArrParent;
        for (int i = 0; i < mNElements; ++i) {
            pmT[i].setArrayParent(pArrParent);
        }
    }
    
    
    /*!
     * \brief signalChange Inverse-recursive function for writing the whole
     *        array to the target file. If this is not the highest-level array,
     *        continue it up the tree.
     */
    void signalChange() {
        if (mpArrParent == 0) {
            // This is the highest array with compound members. Do the write.
            writeAll();
        } else {
            mpArrParent->signalChange();
        }
    }
    
    
    /*!
     * \brief Calling this will write all data stored in local memory in the
     *        members to the target HDF5 file, if it is open.
     */
    void writeAll() {
        if (!mReadDone) {
            readAll();
        }
        if (mpParent == 0) {
            return;
        }
        CPH5IOFacility *pioFacility = mpParent->getIOFacility();
        if (pioFacility == 0) {
            return;
        }
        size_t bufSize = T().getTotalMemorySize()*mNElements;
        char *buf = new char[bufSize];
        char *pBuf = buf;
        try {
            for (int i = 0; i < mNElements; ++i) {
                pmT[i].copyAllAndMove(pBuf);
            }
            H5::CompType thisCompType(mpArrType.get()->getSize());
            thisCompType.insertMember(mName, 0, *mpArrType.get());
            H5::DataType topCompType(mpParent->nestCompTypeIR(thisCompType));
            pioFacility->write(buf, topCompType);
        } catch (...) {
            delete[] buf;
            throw;
        }

        delete[] buf;
    }
    
    
    /*!
     * \brief Reads all members from the target HDF5 file, if it is open, into
     *        the members local memory.
     */
    void readAll() {
        if (mpParent == 0 || mpArrParent != 0) {
            return;
        }
        CPH5IOFacility *pioFacility = mpParent->getIOFacility();
        if (pioFacility == 0) {
            return;
        }
        H5::CompType thisCompType(mpArrType.get()->getSize());
        thisCompType.insertMember(mName, 0, *mpArrType.get());
        H5::DataType topCompType(mpParent->nestCompTypeIR(thisCompType));
        std::vector<char> buf;
        buf.resize(topCompType.getSize());
        pioFacility->read(buf.data(), topCompType);
        char *pBuf = buf.data();
        for (int i = 0; i < mNElements; ++i) {
            pmT[i].latchAllAndMove(pBuf);
        }
        
        mReadDone = true;
    }
    
    
    //TODO document
    CPH5IOFacility *getIoFacility() {
        if (mpParent != 0) {
            return mpParent->getIOFacility();
        }
        return 0;
    }
    
    
    //TODO document
    virtual CPH5TreeNode::CPH5LeafType getLeafType() const override {
        // Arrays are never leaves.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual bool getValIfLeaf(void *p) override {
        return false;
    }
    
    //TODO document
    virtual bool canIndexInto() const override {
        return true;
    }
    
    //TODO document
    virtual CPH5TreeNode *indexInto(int i) override {
        mpParent->readAll(); //TODO does this cause unnecessary reads?
        return dynamic_cast<CPH5TreeNode*>(operator [](i).getTreeNode());
    }
    
    //TODO document
    virtual int getIndexableSize() const override {
        return mNElements;
    }
    
    //TODO document
    CPH5TreeNode::CPH5LeafType getElementType() const {
        // This is an array of compound types.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    int getMemorySizeBelow() const {
        return getSize(); 
    }
    
    //TODO document
    bool readAllBelow(void *p) {
        readAll();
        char *temp = reinterpret_cast<char*>(p);
        copyAndMove(temp);
        return true;
    }
    
    //TODO document
    void *getMemoryLocation() const {
        return 0;
    }
    
    //TODO document
    virtual std::vector<std::string> getChildrenNames() const override {
        return std::vector<std::string>();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string name) const override {
        return 0;
    }
    
    
protected:
    
    CPH5CompType *mpParent;
    std::string mName;
    H5::DataType mBaseType;
	std::shared_ptr<H5::ArrayType> mpArrType;
    bool mReadDone;
    CPH5CompMemberArrayBase *mpArrParent;
    
    std::vector<int> mDatasetIndices;
    
    int mNElements;
    T *pmT;
    
};









/*!
 * \brief This implementation of the CPH5CompMemberArray is a simple pass-
 *        through of the CPH5CompMemberArrayCommon class. The reason that
 *        the functionality of the common class is not simply written into
 *        this class is that we want to create a specialization for the char
 *        implementation of the arrays with support for std::strings.
 */
template<class T, const int nElements>
class CPH5CompMemberArray :
        public CPH5CompMemberArrayCommon<T, IsDerivedFrom<T, CPH5CompType>::Is>
{
	typedef CPH5CompMemberArrayCommon<T, IsDerivedFrom<T, CPH5CompType>::Is> CPH5CompMemberArrayCommonSpec;
public:
    
    
    /*!
     * \brief CPH5CompMemberArray Constructor for implementations not derived
     *                            from a compound type (unless it is desired to
     *                            explicitly override the type derived from
     *                           the template argument).
     * \param parent
     * \param name
     * \param type
     */
    CPH5CompMemberArray(CPH5CompType *parent,
                        std::string name,
                        H5::DataType type)
        : CPH5CompMemberArrayCommonSpec(parent, name, type, nElements)
    {} // NOOP
    
    
    /*!
     * \brief CPH5CompMemberArray Constructor for implementations that are
     *                            derived from a compound type
     * \param parent
     * \param name
     */
    CPH5CompMemberArray(CPH5CompType *parent,
                        std::string name)
        : CPH5CompMemberArrayCommonSpec(parent, name, nElements)
    {} // NOOP
    
    
private:
    
    // These need rejiggering if going to be used, for now private. The assignment
    // operator should perform a write. Any reason the copy constructor should be
    // used?
    /*!
     * \brief operator = Overloaded assignment operator - undefined and private
     *        for now.
     * \param other Other array to assign from.
     * \return Reference to this.
     */
    CPH5CompMemberArray<T, nElements> &operator=
    (const CPH5CompMemberArray<T, nElements> &other)
    ;//{
    //    memcpy(mT, other.mT, sizeof(T)*nElements);
    //    mBaseType = other.mBaseType;
    //    mArrType = other.mArrType;
    //    mName = other.mName;
    //    mpParent = 0;
    //    mReadDone = other.mReadDone;
    //    return *this;
    //}
    
    /*!
     * \brief CPH5CompMemberArray Copy Constructor - undefined and private for
     *        now.
     * \param other Other array to copy from.
     */
    CPH5CompMemberArray(const CPH5CompMemberArray<T, nElements> &other)
    ;//{
    //    memcpy(mT, other.mT, sizeof(T)*nElements);
    //    mBaseType = other.mBaseType;
    //    mArrType = other.mArrType;
    //    mName = other.mName;
    //    mpParent = 0;
    //}

};




/*!
 * \brief The CPH5CompMemberArray<char, nElements> class is a special subclass
 *        of the CPH5CompMemberArrayCommon functionality for chars with some
 *        extra overloads added to translate to and from std::strings, since
 *        they are the most common use of an array of chars.
 */
template<const int nElements>
class CPH5CompMemberArray<char, nElements>
        : public CPH5CompMemberArrayCommon<char, IS_NOT_DERIVED>
{
public:
    
    /*!
     * \brief CPH5CompMemberArray Constructor, see CPH5CompMemberArrayCommon
     *        constructor.
     * \param parent CompType subclass that this member should belong to.
     * \param name Name to assign to this member in the target HDF5 file.
     * \param type Base type of this array to assign to this member in the
     *        target HDF5 file.
     */
    CPH5CompMemberArray(CPH5CompType *parent,
                        std::string name,
                        H5::DataType type)
        : CPH5CompMemberArrayCommon(parent, name, type, nElements)
    {} // NOOP
    
    
    /*!
     * \brief operator std::string Special overloaded typecast operator to a 
     *        std::string. Uses the read function to read all of the chars into
     *        a c-style string and then creates and returns a std::string out
     *        of that data.
     */
    operator std::string()
    {
        char *buf = new char[getSize()+1];
        std::string ret;
        try {
            memset(buf, 0, getSize()+1);
            read(buf);
            ret = buf;
        } catch (...) {
            delete[] buf;
            throw;
        }

        delete[] buf;
        return ret;
    }
    
    
    /*!
     * \brief operator = Special overloaded assignment operator for a
     *        std::string. Calls the overloaded write function for std::string.
     * \param other String containing data to source from.
     */
    void operator=(const std::string &other) {
        write(other);
    }
    
    
    /*!
     * \brief Special overloaded write function accepting a std::string instead
     *        of a raw buffer. Performs the same actions as the non-specialized
     *        version of write.
     * \param str String with data to write to target HDF5 file.
     */
    void write(const std::string &str) {
        char *buf = new char[getSize()];
        
        try {
            memset(buf, 0, getSize());
            if (str.length() < getSize()) {
                // The string to write is smaller than the container can hold.
                // All is good.
                memcpy(buf, str.c_str(), str.length());
            } else {
                // The string length is greater than or equal to.
                memcpy(buf, str.c_str(), getSize());
            }
            memcpy(pMT, buf, nElements);
            if (mpParent != 0) {
                CPH5IOFacility *pIO = mpParent->getIOFacility();
                if (pIO != 0) {
                    H5::CompType h5CompType(mpArrType.get()->getSize());
                    h5CompType.insertMember(mName, 0, *mpArrType.get());
                    pIO->write(pMT, mpParent->nestCompTypeIR(h5CompType));
                    //pIO->write(mT, h5CompType);
                } else if (mpArrParent != 0) {
                    mpArrParent->signalChange();
                }
            }
        } catch (...) {
            delete[] buf;
            throw;
        }
        
        delete[] buf;
    }
    
    
    /*!
     * \brief getStrOfValue Gets the string representation of this array. 
     *        Since this specialization is for an array of chars, no other
     *        conversion is needed.
     * \return String of value.
     */
    std::string getStrOfValue() {
        char locT[nElements+1];
        memset(locT, 0, nElements+1);
        read(locT);
        return locT;
    }
    
    
};





// NON-INHERITED SPECIALIZATION method
template<typename T, const int I>
const T& CPH5CompMemberBaseThru<T, I>::get() const {
    if (mpParent != 0) {
        CPH5IOFacility *pIO = mpParent->getIOFacility();
        if (pIO != 0) {
            H5::CompType h5CompType(sizeof(T));
            h5CompType.insertMember(mName, 0, mType);
            pIO->read(&mT, mpParent->nestCompTypeIR(h5CompType));
        }
    }
    return mT;
}

// NON-INHERITED SPECIALIZATION method
template<typename T, const int I>
void CPH5CompMemberBaseThru<T, I>::operator=(const T &other) {
    mT = other;
    if (mpParent != 0) {
        CPH5IOFacility *pIO = mpParent->getIOFacility();
        if (pIO != 0) {
            H5::CompType h5CompType(sizeof(T));
            h5CompType.insertMember(mName, 0, mType);
            pIO->write(&mT, mpParent->nestCompTypeIR(h5CompType));
            //pIO->write(&mT, h5CompType);
        } else if (mpArrParent != 0) {
            mpArrParent->signalChange();
        }
    }
}

// INHERITED SPECIALIZATION from CPH5CompType method
template<class T>
CPH5IOFacility *CPH5CompMemberBaseThru<T, IS_DERIVED>::getIOFacility() const {
    if (mpParent != 0) {
        return mpParent->getIOFacility();
    }
    return 0;
}

 
// INHERITED SPECIALIZATION from CPH5CompType method
template<class T>
H5::CompType CPH5CompMemberBaseThru<T, IS_DERIVED>::nestCompTypeIR(H5::CompType leaf) {
    H5::CompType ret(leaf.getSize());
    ret.insertMember(mName, 0, leaf);
    if (mpParent != 0) {
        return mpParent->nestCompTypeIR(ret);
    } else {
        return ret;
    }
}


template<class T>
// INHERITED SPECIALIZATION from CPH5CompType method
void CPH5CompMemberBaseThru<T, IS_DERIVED>::setArrayParent(CPH5CompMemberArrayBase *pArrParent) {
    CPH5CompType::setArrayParent(pArrParent);
}


#endif // CPH5COMPTYPE_H
