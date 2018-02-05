////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5DATASET_H
#define CPH5DATASET_H

#include "cph5utilities.h"
#include "cph5group.h"
#include "cph5comptype.h"


#ifdef _MSC_VER
#pragma warning(disable: 4355 4706 4115 4100 4201 4214 4054 4244 4267)
#endif /* _MSC_VER */


// C++11 upgrade list:
//   Delete overloaded read/write/readRaw/writeRaw function that pass
//     around the wrong type.


/*!
 * \brief The CPH5DatasetBase class is a potentially specialized
 *        base class for the CPH5Dataset class. It's specialization
 *        is dependent on whether T is inherited from CompType, and
 *        whether the dataset object is order 0 (scalar) or not.
 * 
 * If this particular implementation is used instead of one of the
 * specialized versions, it means that the parent class is templated
 * with a non-compound type, and that this particular object is for
 * a non-0 order (more than just a single element). Implicitly, the
 * CPH5DatasetBase<T, 0, i> will be implemented for the order 0
 * object. The first template parameter is always the C++ type that
 * the Dataset should be treated as in code. The second template
 * parameter must be a constant integer for the number of dimensions
 * of the particular dataset. The third parameter is not used by the
 * user and is given to DatasetBase by the Dataset class, through 
 * the IsDerivedFrom class to signify whether the first parameter
 * is derived from CPH5CompType (1 means it is, anything else means
 * it is not).
 * 
 * The reason that such specializations are needed is that the read
 * and write functionality must be different for atomic types versus
 * compound types. Additionally, order-0 objects must provide the 
 * assignment operator and cast operator. Thus we have four options
 * for DatasetBase: non-inherited order 1+, non-inherited order 0,
 * inherited order 1+, inherited order 0.
 */
template<class T, const int nDims, const int>
class CPH5DatasetBase
{
    // NON-INHERITED SPECIALIZATION, order 1+
public:
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases) and an H5::DataType since this is a non-
     *        inherited implementation. This is a non-scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     * \param type H5::DataType to use (normally an H5::PredType).
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility,
                    H5::DataType type)
    {
        mpIOFacility = pioFacility;
        mType = type;
    }
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases) but no H5::DataType, instead using the
     *        CPH5TypeProxy mappings. This is a non-scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility)
    {
        mpIOFacility = pioFacility;
        mType = static_cast<H5::DataType>(CPH5TypeProxy<T>());
    }
    
    /*!
     * \brief Reads data at this dimension level into the given buffer from the
     *        target HDF5 file.
     * \param buf T pointer to buffer that must be large enough to accept the
     *        data.
     */
    void read(T *buf) {
        if (mpIOFacility != 0) {
            mpIOFacility->read(buf);
        }
    }
    
    /*!
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file.
     * \param src T pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void write(const T *src) {
        mpIOFacility->write(src);
    }
    
    
    /*!
     * \brief readRaw Reads data at this dimension level into the given buffer
     *        from the target HDF5 file. For this specialization, is identical
     *        to the read function.
     * \param buf Pointer to buffer that must be large enough to accept the
     *        data.
     */
    void readRaw(void *buf) {
        read((T*)buf);
    }
    
    
    /*!
     * \brief writeRaw Writes data at this dimension level from the given
     *        buffer to the target HDF5 file. For this specialization, is
     *        identical to the write function.
     * \param src Pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void writeRaw(const void *src) {
        write((T*)src);
    }
    
    
protected:
    
    H5::DataType mType;
    
private:
    
    // Disable the move and copy constructors
    CPH5DatasetBase(CPH5DatasetBase &&other);
    CPH5DatasetBase &operator=(CPH5DatasetBase &&other);
    CPH5DatasetBase(CPH5DatasetBase &other);
    
    CPH5IOFacility *mpIOFacility;
};



/*!
 * \brief The CPH5DatasetBase<T, 0, i> class is a specialization
 *        of the CPH5DatasetBase template class, specific for
 *        a non-inherited order 0 object.
 */
template<class T, const int i>
class CPH5DatasetBase<T, 0, i>
{
    // NON-INHERITED SPECIALIZATION, ORDER 0
public:
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases) and an H5::DataType since this is a non-
     *        inherited implementation. This is a scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     * \param type H5::DataType to use (normally an H5::PredType).
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility,
                    H5::DataType type)
    {
        mpIOFacility = pioFacility;
        mType = type;
    }
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases) but no H5::DataType, instead using the
     *        CPH5TypeProxy mappings. This is a scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility)
    {
        mpIOFacility = pioFacility;
        mType = static_cast<H5::DataType>(CPH5TypeProxy<T>());
    }
    
    /*!
     * \brief operator T Cast operator to the type for which this class is
     *        templated so that it can be read using basic assignment. I.e.
     *        T a = *this;
     */
    operator T() {
        read(0);
        return mTBuf;
    }
    
    
    /*!
     * \brief Reads data at this dimension level into the given buffer from the
     *        target HDF5 file.
     * \param buf T pointer to buffer that must be large enough to accept the
     *        data.
     */
    void read(T *buf) {
        mpIOFacility->read(&mTBuf);
        if (buf != 0) {
            *buf = mTBuf;
        }
    }
    
    /*!
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file.
     * \param src T pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void write(const T* src) {
        mTBuf = *src;
        mpIOFacility->write(src);
    }
    
    
    /*!
     * \brief readRaw Reads data at this dimension level into the given buffer
     *        from the target HDF5 file. For this specialization, is identical
     *        to the read function.
     * \param buf Pointer to buffer that must be large enough to accept the
     *        data.
     */
    void readRaw(void *buf) {
        read((T*)buf);
    }
    
    
    /*!
     * \brief writeRaw Writes data at this dimension level from the given
     *        buffer to the target HDF5 file. For this specialization, is
     *        identical to the write function.
     * \param src Pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void writeRaw(const void *src) {
        write((T*)src);
    }
    
    
    /*!
     * \brief operator = Overloaded assignment operator to allow assignment of
     *        this element equivalent to a write, since this is a scalar-order
     *        dataset.
     * \param rhs Value to write.
     */
    void operator=(T &rhs) {
        write(&rhs);
    }
    
    int getTotalMemorySize() const {
        return sizeof(T);
    }
    
    
    // Note that these are NOT virtual - this is by design.
    
    //TODO document
    CPH5TreeNode::CPH5LeafType getLeafType() const {
        return static_cast<CPH5TreeNode::CPH5LeafType>(CPH5TreeNode::IsLeaf<T>::Get);
    }
    
    //TODO document
    bool getValIfLeaf(void *p) {
        if (getLeafType() != CPH5TreeNode::LT_IS_NOT_LEAF) {
            readRaw(p);
            return true;
        }
        return false;
    }
    
    
    //TODO document
    CPH5TreeNode::CPH5LeafType getElementType() const {
        // This might be counter-intuitive, but scalar datasets do not
        // have an element type, because they are not an array.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    int getMemorySizeBelow() const {
        return sizeof(T);
    }
    
    //TODO document
    bool readAllBelow(void *p) {
        return getValIfLeaf(p);
    }
    
    //TODO document
    void *getMemoryLocation() const {
        return &mTBuf;
    }
    
    //TODO document
    std::vector<std::string> getChildrenNames() const {
        // The template argument is not inherited from CPH5CompType, so there
        // is no children.
        return std::vector<std::string>();
    }
    
    //TODO document
    CPH5TreeNode *getChildByName(std::string name) const {
        // The template argument is not inherited from CPH5CompType, so there
        // is no children.
        return 0;
    }
    
    
    
protected:
    
    H5::DataType mType;
    
private:
    
    // Also disable the move constructors
    CPH5DatasetBase(CPH5DatasetBase &&other);
    CPH5DatasetBase &operator=(CPH5DatasetBase &&other);
    
    CPH5IOFacility *mpIOFacility;
    
    mutable T mTBuf;
};


/*!
 * \brief The CPH5DatasetBase<T, i, 1> class is a specialization
 *        of the CPH5DatasetBase template class, specific for
 *        an inherited order 1+ object.
 */
template<class T, const int I>
class CPH5DatasetBase<T, I, IS_DERIVED>
{
    // INHERITED SPECIALIZATION! ORDER 1+
public:
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases), but no type since this specialization
     *        is for compound types the type will be derived from the template
     *        parameter. This is a non-scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility = 0)
    {
        mpIOFacility = pioFacility;
        
        mType = T().getCompType();
    }
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases), in addition to a type, if for some
     *        reason the user wanted to pass in a type explicitly. This is
     *        used by the CPH5Dynamic capability since type cannot be known
     *        at compile time. This is an order 1+ rank dataset.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     * /param type Compound type to use when accessing target file.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility,
                    H5::CompType type)
    {
        mpIOFacility = pioFacility;
        
        mType = type;
    }
    
    /*!
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file. T is expected to be an array of CPH5CompType,
     *        large enough for all elements at and below this level to be
     *        read from.
     * \param items Pointer to array to source data from. Must be large enough
     *        to read all necessary data from.
     */
    // Future enhancement: figure out how to do this without making two copies.
    void write(const T *items) {
        if (mpIOFacility != 0) {
            int nElements = mpIOFacility->getNumLowerElements();
            char *pBuf = new char[items->getTotalMemorySize()*nElements];
            char *pBufr = pBuf;
            for (int i = 0; i < nElements; ++i) {
                items[i].copyAllAndMove(pBufr);
            }
            try {
                mpIOFacility->write(pBuf);
            } catch (...) {
                delete[] pBuf;
                throw;
            }
            delete[] pBuf;
        }
    }
    
    /*!
     * \brief Reads data at this dimension level into the given array from the
     *        target HDF5 file.
     * \param items Pointer to array to store data into. Must be large enough
     *        to store all necessary data into.
     */
    // Future enhancement: figure out how to do this without making two copies.
    void read(T *items) {
        if (mpIOFacility != 0) {
            int nElements = mpIOFacility->getNumLowerElements();
            char *pBuf = new char[items->getTotalMemorySize()*nElements];
            try {
                mpIOFacility->read(pBuf);
            } catch (...) {
                delete[] pBuf;
                throw;
            }
            char *pBufr = pBuf;
            for (int i = 0; i < nElements; ++i) {
                items[i].latchAllAndMove(pBufr);
            }
            delete[] pBuf;
            
        }
    }
    
    
    /*!
     * \brief readRaw Reads data at this dimension level into the given buffer
     *        from the target HDF5 file. For this specialization, differs from
     *        read function because does not expect pointer to point to a
     *        compound type and reads the data as simple binary.
     * \param buf Pointer to buffer that must be large enough to accept the
     *        data.
     */
    void readRaw(void *buf) {
        if (mpIOFacility != 0) {
            mpIOFacility->read(buf);
        }
    }
    
    
    /*!
     * \brief writeRaw Writes data at this dimension level from the given
     *        buffer to the target HDF5 file. For this specialization, differs
     *        from write function because does not expect the pointer to be to
     *        a compound type and writes the data as simple binary.
     * \param src Pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void writeRaw(const void *src) {
        if (mpIOFacility != 0) {
            mpIOFacility->write(src);
        }
    }
    
    
protected:
    
    H5::DataType mType;
    
private:
    
    CPH5IOFacility *mpIOFacility;
};


/*!
 * \brief The CPH5DatasetBase<T, 0, 1> class is a specialization
 *        of the CPH5DatasetBase template class, specific for
 *        an inherited order 0 object.
 * 
 * Unlike the non-inherited scalar specialization, there is no cast operator
 * overload because this class is set up to inherit itself from T.
 */
template<class T>
class CPH5DatasetBase<T, 0, IS_DERIVED> : public T
{
    // INHERITED SPECIALIZATION! ORDER 0
public:
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases), but no type since this specialization
     *        is for compound types the type will be derived from the template
     *        parameter. This is a scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility)
    {
        this->setIOFacility(pioFacility);
        mpIOFacility = pioFacility;
        mType = T().getCompType();
        mPrevFirstOrderIndex = -1;
    }
    
    /*!
     * \brief CPH5DatasetBase constructor, accepts CPH5IOFacility pointer (as
     *        do all CPH5DatasetBases), in addition to a type, if for some
     *        reason the user wanted to pass in a type explicitly. This is
     *        used by the CPH5Dynamic capability since type cannot be known
     *        at compile time. This is a scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     * /param type Compound type to use when accessing target file.
     */
    CPH5DatasetBase(CPH5IOFacility *pioFacility,
                    H5::CompType type)
    {
        this->setIOFacility(pioFacility);
        mpIOFacility = pioFacility;
        mType = type;
        mPrevFirstOrderIndex = -1;
    }
    
    /*!
     * \brief Reads data at this dimension level into the given item from the
     *        target HDF5 file.
     * \param item Pointer to item to store data into.
     */
    // Future enhancement: figure out how to do this without making two copies.
    void read(T *item) {
        this->readAll();
        
        char *pBuf = new char[this->getTotalMemorySize()];
        char *pBufr = pBuf;
        try {
            this->copyAllAndMove(pBufr);
            pBufr = pBuf;
            item->latchAllAndMove(pBufr);
        } catch (...) {
            delete[] pBuf;
            throw;
        }

        delete[] pBuf;
        
        // This code (along with the readAll line above) was used when
        // this function expected everything to be stored in flat buffers.
        //char *p = static_cast<char*>(buf);
        //copyAllAndMove(p);
    }
    
    /*!
     * \brief Writes data at this dimensions level from the given item to the
     *        target HDF5 file.
     * \param item Pointer to item to source data from.
     */
    // Future enhancement: figure out how to do this without making two copies.
    void write(const T *item) {
        // This code (along with the writeAll line below) was used when
        // this function expected everything to be stored in flat buffers.
        //char *p = static_cast<char*>(buf);
        //latchAllAndMove(p);
        
        char *pBuf = new char[this->getTotalMemorySize()];
        char *pBufr = pBuf;
        try {
            item->copyAllAndMove(pBufr);
            pBufr = pBuf;
            this->latchAllAndMove(pBufr);
        } catch (...) {
            delete[] pBuf;
            throw;
        }

        delete[] pBuf;
        this->writeAll();
    }
    
    
    /*!
     * \brief readRaw Reads data at this dimension level into the given buffer
     *        from the target HDF5 file. For this specialization, differs from
     *        read function because does not expect pointer to point to a
     *        compound type and reads the data as simple binary.
     * \param buf Pointer to buffer that must be large enough to accept the
     *        data.
     */
    void readRaw(void *buf) {
        this->readAll();
        char *p = reinterpret_cast<char*>(buf);
        this->copyAllAndMove(p);
    }
    
    
    /*!
     * \brief writeRaw Writes data at this dimension level from the given
     *        buffer to the target HDF5 file. For this specialization, differs
     *        from write function because does not expect the pointer to be to
     *        a compound type and writes the data as simple binary.
     * \param src Pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void writeRaw(const void *src) {
        void *nsrc = const_cast<void*>(src);
        char *p = reinterpret_cast<char*>(nsrc);
        this->latchAllAndMove(p);
        this->writeAll();
    }
    
    
    /*!
     * \brief operator = Overloaded assignment operator to allow assignment
     *        from another T type (which would be inherited from CPH5CompType).
     * \param rhs CPH5CompType derivative to take values from and write them.
     */
    void operator=(T &rhs) {
        CPH5IOFacility *temp = rhs.getIOFacility();
        rhs.setIOFacility(mpIOFacility);
        rhs.writeAll();
        rhs.setIOFacility(temp);
    }
    
    // Note that these are all virtual but they are not override.
    // This is by design.
    
    //TODO document
    virtual CPH5TreeNode::CPH5LeafType getLeafType() const {
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual bool getValIfLeaf(void *p) {
        return false;
    }
    
    //TODO document
    virtual CPH5TreeNode::CPH5LeafType getElementType() const {
        // Scalar datasets do not have an element type because they
        // are not arrays.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    virtual void *getMemoryLocation() const {
        return 0;
    }

    //TODO document
    virtual std::vector<std::string> getChildrenNames() const {
        // The template argument is inherited from CPH5CompType, so there
        // are children.
        return T::getTreeNode()->getChildrenNames();
    }
    
    //TODO document
    virtual CPH5TreeNode *getChildByName(std::string name) const {
        // The template argument is inherited from CPH5CompType, so there
        // are children.
        return T::getTreeNode()->getChildByName(name);
    }
    
    
protected:
    
    H5::DataType mType;
    
private:
    
    CPH5IOFacility *mpIOFacility;
    int mPrevFirstOrderIndex;
};




/*!
 * \brief The CPH5DatasetIdBase class is used simply for dynamic_cast testing
 *        of whether a group member is a dataset.
 */
class CPH5DatasetIdBase
{
public:
   virtual std::vector<int> getDims() const = 0;
   virtual H5::DataSet *getDataSet() const = 0;
};






/*!
 * \brief The CPH5Dataset class defines a multi-dimensional dataset and allows
 *        access to individual elements or array subsets.
 * 
 * The first template parameter is the type that the object should be treated
 * as an array of in C++ code. The second parameter must be a constant integer
 * for the number of dimensions in the array - ie. 3 would mean a 3 dimensional
 * array, 2 would be a matrix, 1 a list and 0 a single element.
 * 
 * The CPH5Dataset class is a recursive template - meaning any implementation
 * of it will cause the compiler to generate implementations of 0-33 (which are
 * both specialized in order to prevent infinite recursion). This # is referred
 * to as the 'order', and 32 is the max number of dimensions supported by the 
 * HDF5 specification hence the max order of 33 (33 has private constructors
 * only). Additionally, orders one above and one below are friended so that all
 * Dataset internal functionality can be private.
 * 
 * A 'root order' Dataset is the dataset which is implemented by the user and
 * not as a result of the template system. It is the object with which the user
 * has to interact with in order to access the sub-elements of the dataset.
 */
template<class T, const int nDims>
class CPH5Dataset : 
        public CPH5GroupMember,
        public CPH5AttributeHolder,
        // SFINAE
        public CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>,
        public CPH5DatasetIdBase
{
    
public:
    
    /*!
     * \brief Public constructor for root-order objects that are
     *        instantiated with a non-compound type. This means that the H5::
     *        DataType (usually a PredType) must be passed in. Attempting to
     *        use this constructor with a compound type in the template list
     *        will result in a compiler error.
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     * \param type The HDF5 DataType to use when storing the non-compound 
     *             element.
     */
    CPH5Dataset(CPH5Group *parent,
              std::string name,
              H5::DataType type)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility,
                          type),
          mNextDim(this, type),
          mpDataSet(0),
          mDimsSet(false),
          mChunksSet(false),
          mDeflateSet(false)
    {
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        parent->registerChild(this);
        
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    /*!
     * \brief Public constructor for root-order objects that are
     *        instantiated with a compound type. This means that the H5::
     *        CompType passed in will be used instead of the type derived
     *        from the template argument (if it is compound).
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     * \param type The HDF5 CompType to use when storing the non-compound 
     *             element.
     */
    CPH5Dataset(CPH5Group *parent,
              std::string name,
              H5::CompType type)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility,
                          type),
          mNextDim(this, type),
          mpDataSet(0),
          mDimsSet(false),
          mChunksSet(false),
          mDeflateSet(false)
    {
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        parent->registerChild(this);
        
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    /*!
     * \brief Public constructor for root-order objects that are
     *        instantiated <i>with a compound datatype</i>. This means that the
     *        template parameter should be a class/struct inherited from
     *        CPH5CompType. This constructur must be used with compound types,
     *        disallowing the explicit passing of an H5::DataType into the 
     *        dataset class - as it will be derived from the given compound
     *        type.
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     */
    CPH5Dataset(CPH5Group *parent,
              std::string name)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility),
          mNextDim(this),
          mpDataSet(0),
          mDimsSet(false),
          mChunksSet(false),
          mDeflateSet(false)
    {
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        parent->registerChild(this);
        
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    /*!
     * \brief Destructor. Calls closeR and deletes the
     *        CPH5IOFacility member if it has been created.
     */
    virtual ~CPH5Dataset() {
        closeR();
        if (mpGroupParent != 0 && mpIOFacility != 0) {
            delete mpIOFacility;
            mpIOFacility = 0;
        }
    }
    
    /*!
     * \brief Recursive open function. Generally called by the parent of this 
     *        dataset object, whether that be a group or another dataset. The R
     *        suffix to the function name means this is a Recursive 
     *        function - calls to openR higher in the tree will recurse down
     *        the tree to this object, and this object calls it's children's 
     *        openR function. 
     * \param create A flag for whether to create the dataset or open it.
     */
    void openR(bool create) {
        if (!mDimsSet && create) {
            // Future: proper error. For now just return
            return;
        }
        if (mpGroupParent == 0)
            return;
        if (create) {
            H5::DataSpace space(nDims, mDims, mMaxDims);
            if (mChunksSet) {
                mpDataSet = mpGroupParent->createDataSet(mName,
                                                         this->mType,
                                                         space,
                                                         mPropList);
            } else {
                mpDataSet = mpGroupParent->createDataSet(mName, this->mType, space);
            }
        } else {
            mpDataSet = mpGroupParent->openDataSet(mName);
            H5::DataSpace filespace(mpDataSet->getSpace());
            if (filespace.getSimpleExtentNdims() != nDims) {
                // Future: proper error. For now just return
                return;
            }
            filespace.getSimpleExtentDims(mDims, mMaxDims);
            mDimsSet = true;
        }
        if (mChildren.size() > 0) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->openR(create);
            }
        }
    }
    
    /*!
     * \brief Recursive close function. Calls closeR of all registered children
     *        and then deletes the member dataset if it is created AND this
     *        is a root-order object. 
     */
    void closeR() {
        if (mChildren.size() > 0) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->closeR();
            }
        }
        if (mpDataSet != 0 && mpGroupParent != 0) {
            mpDataSet->close();
            delete mpDataSet;
            mpDataSet = 0;
        }
    }
    
    /*!
     * \brief Indexing operator for use if this dataset has non-scalar 
     *        dimensions. Returns a reference to the next lower order dataset.
     * \param ind The index to use. Undefined behavior will result if the index
     *        is out of bounds. 
     * \return A reference to a dataset with one lower order. For example: if
     *         this is called on the highest order of a two-dimensional array,
     *         the return will be a reference to a CPH5Dataset object with the
     *         same type but with order 1 - i.e. a row in the 2D array.
     */
    CPH5Dataset<T, nDims-1> &operator[](int ind) {
        if (mpGroupParent != 0) {
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        mpIOFacility->addIndex(ind);
        
        return mNextDim;
    }
    
    
    /*!
     * \brief Returns the size of this dimension, as either set by the user
     *        with setDimensions (if file is being created) or as read from
     *        the target HDF5 file if opened.
     * \return An integer value for the size of the dimension.
     */
    int getDimSize() const {
        return getDimSizeIR(0);
    }
    
    
    /*!
     * \brief Returns the maxmium size of this dimension, as either set by the
     *        user with setDimensions (if file is being created) or as read
     *        from the target HDF5 file if opened.
     * \return An integer value for the max size of the dimension.
     */
    int getMaxDimSize() const {
        return getMaxDimSizeIR(0);
    }
    
    
    /*!
     * \brief getDims Returns vector of dimensions for this dataset.
     * \return Vector of dimensions for this dataset.
     */
    std::vector<int> getDims() const {
       std::vector<int> ret;
       for (int i = 0; i < nDims; ++i) {
          ret.push_back(getDimSizeIR(i));
       }
       return ret;
    }
    
    
    /*!
     * \brief getMaxDims Returns vector of maximum dimensions for this dataset.
     * \return Vector of maximum dimensions for this dataset.
     */
    std::vector<int> getMaxDims() const {
       std::vector<int> ret;
       for (int i = 0; i < nDims; ++i) {
          ret.push_back(getMaxDimSizeIR(i));
       }
       return ret;
    }
    
    
    /*!
     * \brief Sets the dimensions of a dataset if it is to be created as part
     *        of a new HDF5 file. This should not be called on a non
     *        root-order object.
     * \param dims An array of hsize_t with the number of elements that this
     *        root-order dataset object was created with, representing the
     *        starting size of the dataset dimensions.
     * \param maxDims An array of hsize_t with the maximum size of each
     *        dimension. For unlimited size datasets, use the value of
     *        H5S_UNLIMITED.
     */
    void setDimensions(hsize_t dims[nDims],
                       hsize_t maxDims[nDims]) {
        memcpy(mDims, dims, nDims*sizeof(hsize_t));
        memcpy(mMaxDims, maxDims, nDims*sizeof(hsize_t));
        mDimsSet = true;
    }
    
    /*!
     * \brief Sets the chunk size to use to store the memory for this dataset
     *        in the target HDF5 file. This should not be called on a non
     *        root-order object. Reference the HDF5 online documentation for
     *        the best application of this. Note that datasets with
     *        unlimited maximum size <i>require</i> the chunk size to be set.
     * \param chunkDims An array of hsize_t with the chunk size to use for
     *        each dimension.
     * 
     * For example: an image cube with images of size 1024 by 1024, and 1800 
     * total images would be a 3-dimensional dataset with dimensions 1800, 
     * 1024, and 1024. If the most common memory access is going to be to read
     * or write a single frame - 1024 x 1024, then the chunk size should be set
     * to [1, 1024, 1024]. This will increase file I/O efficiency. 
     */
    void setChunkSize(hsize_t chunkDims[nDims]) {
        mPropList.setChunk(nDims, chunkDims);
        mChunksSet = true;
    }
    
    /*!
     * \brief Sets the compression to use to store memory for this dataset
     *        in the target HDF5 file. This should not be called on a non
     *        root-order object. Reference the HDF5 online documentation for
     *        the best application of this. Note that datasets with
     *        compression set need their chunk size to be set.
     * \param level Integer with the level of compression (1-9) to use
     *
     *
     * */
    void setDeflateLevel(int level) {
        mPropList.setDeflate(level);
        mDeflateSet = true;
    }

    /*!
     * \brief Set the fill value for the dataset. The value needs to be convertible
     *        into the dataset type for this dataset. Reference the HDF5 online documentation for
     *        the best application of this.
     * \param fillVal value to set all the element values by default
     *
     *
     * */
    void setFillValue(T fillVal) {
        mPropList.setFillValue(this->mType, &fillVal);
    }

    /*!
     * \brief Writes data from a pointer to an array of type T to
     *        the target HDF5 file. The object that this is being
     *        called in reference to contains the location information for
     *        where to write. The array of data given must contain enough data
     *        to fill the dimensions of this dataset. This can be called on any
     *        order dataset. The data ifself must be an array of T. If T is a 
     *        compound type, will pull the binary data out of those elements
     *        instead of treating like a raw buffer.
     * \param src The pointer to the array of data to write.
     * 
     * Under the hood, this function initialized the CPH5IOFacility for the
     * dataset tree, and then calls the base class write function - which could
     * be one of the four template specializations of CPH5DatasetBase.
     */
    void write(const T *src) {
        // Can be used at every level
        if (mpGroupParent != 0) {
            // Root level
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>::write(src);
    }
    
    
    /*!
     * \brief Writes data from a pointer to a block of binary data to
     *        the target HDF5 file. The object that this is being
     *        called in reference to contains the location information for
     *        where to write. The block of data given must contain enough data
     *        to fill the dimensions of this dataset. This can be called on any
     *        order dataset. The data ifself must be a raw buffer. Passing in a
     *        pointer to an array of actual CPH5CompType objects doesn't work.
     * \param src The pointer to the block of data to write.
     * 
     * Under the hood, this function initialized the CPH5IOFacility for the
     * dataset tree, and then calls the base class write function - which could
     * be one of the four template specializations of CPH5DatasetBase.
     */
    void writeRaw(const void *src) {
        // Can be used at every level
        if (mpGroupParent != 0) {
            // Root level
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>::writeRaw(src);
    }
    
    
    
    /*!
     * TODO document
     *
     * 
     * 
     */
    void writeRawStartingAt(int ind, const void *src) {
        // Can be used at every level
        if (mpGroupParent != 0) {
            // Root level
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        mpIOFacility->writeWithOffset(ind, src);
    }
    
    
    
    
    /*!
     * \brief Reads data from the HDF5 file into a block of memory. The pointer
     *        must point to an array of T large enough to fit all the data. 
     *        Performs similar action to the write function. The data ifself
     *        must be an array of T. If T is a compound type, the data will be
     *        stored into the elements memory properly instead of treating
     *        the destination like a raw buffer.
     * \param dst Pointer to array of T elements to read data into.
     */
    void read(T *dst) {
        if (mpGroupParent != 0) {
            // Root level
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>::read(dst);
    }
    
    
    
    /*!
     * \brief Reads data from the HDF5 file into a block of memory. The pointer
     *        must point to a block large enough to accomodate all the data. 
     *        Performs similar action to the write function.  The data ifself
     *        must be a raw buffer. Passing in a pointer to an array of actual
     *        CPH5CompType objects doesn't work.
     * \param dst Pointer to block of memory to read data into.
     */
    void readRaw(void *dst) {
        if (mpGroupParent != 0) {
            // Root level
            mpIOFacility->init(mpDataSet,
                               this->mType,
                               nDims,
                               mDims);
        }
        CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>::readRaw(dst);
    }
    
    
    
    /*!
     * \brief Returns the total number of elements currently allocated in the
     *        target HDF5 file (whether it has actually been written or not)
     *        below this point in the dataset tree. 
     * \return Number of elements with the specified type (from the template)
     *         that exist below this point in the dataset tree.
     */
    int getTotalNumElements() const {
        std::vector<int> dims = getDims();
        int ret = dims[0];
        for (int i = 1; i < nDims; ++i) {
            ret = ret * dims[i];
        }
        return ret;
    }
    
    
    /*!
     * \brief Returns a pointer to the H5::DataSet object maintained by this
     *        CPH5Dataset tree, or 0 if one has not been created yet (the
     *        file has not been opened or created).
     * \return Pointer to H5::DataSet object on which this CPH5Dataset tree
     *         operates, or 0 if it has not been created yet.
     */
    H5::DataSet *getDataSet() const {
        if (mpGroupParent != 0) {
            return mpDataSet;
        } else if (mpDimParent != 0) {
            return mpDimParent->getDataSet();
        } else {
            // Future: proper error. For now just return
            return 0;
        }
    }
    
    
    /*!
     * \brief Creates an H5::Attribute attached to this dataset in the target
     *        HDF5 file. If the file has not been opened or created, does
     *        nothing and returns 0;
     * \param name The name to give to the attribute visible in the target HDF5
     *        file.
     * \param dataType The H5::DataType to give to the attribute visible in the
     *        target HDF5 file.
     * \param space The H5::DataSpace to use when defining the dimensions of
     *        the attribute in the target HDF5 file.
     * \return A pointer to the newly created attribute, or 0 if one could not 
     *         be created.
     */
    H5::Attribute *createAttribute(std::string name,
                                   H5::DataType dataType,
                                   H5::DataSpace space) {
        if (mpGroupParent != 0) {
            // Root level
            if (mpDataSet != 0) {
                return new H5::Attribute(mpDataSet->createAttribute(name,
                                                                    dataType,
                                                                    space));
            } else {
                return 0;
            }
        }
        return mpDimParent->createAttribute(name, dataType, space);
    }
    
    
    /*!
     * \brief Opens an H5::Attribute with the given name attached to this
     *        dataset object in the target HDF5 file and returns a pointer to
     *        it, or zero if the attribute could not be opened.
     * \param name Name of attribute to open.
     * \return Pointer to attribute if opened, or 0 if could not be opened.
     */
    H5::Attribute *openAttribute(std::string name) {
        if (mpGroupParent != 0) {
            // Root level
            if (mpDataSet != 0) {
                return new H5::Attribute(mpDataSet->openAttribute(name));
            } else {
                return 0;
            }
        }
        return mpDimParent->openAttribute(name);
    }
    
    
    /*!
     * \brief Should only be called by CPH5Attribute children of this dataset
     *        object to add themselves to the parent-child tree.
     * \param child Pointer to CPH5Attribute child to register.
     */
    void registerAttribute(CPH5AttributeInterface *child) {
        if (mpGroupParent != 0) {
            // Root level
            mChildren.push_back(child);
        } else {
            return mpDimParent->registerAttribute(child);
        }
    }
    
    
    /*!
     * \brief Should only be called by CPH5Attribute children of the dataset
     *        object to remove itself from the parent-child tree. Usually
     *        during destruction.
     * \param child Pointer to CPH5Attribute child to un-register.
     * 
     * This function may be totally unnecessary as natural object destruction
     * should work just as well.
     */
    void unregisterAttribute(const CPH5AttributeInterface *child) {
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            if ((*it) == child) {
                it = mChildren.erase(it);
            }
        }
        if (mpDimParent != 0) {
            mpDimParent->unregisterAttribute(child);
        }
    }
    
    
    /*!
     * \brief Extends this dataset if it is extendible. I.e. the maximum
     *        dimension of this order was set to H5S_UNLIMITED using the
     *        setDimensions function.
     * \param numTimes How many elements to extend the dataset by.
     */
    void extend(int numTimes) {
        extendIR(0, numTimes);
    }
    
    /*!
     * \brief Extends this dataset if it is extendible. I.e. the maximum
     *        dimension of this order was set to H5S_UNLIMITED using the
     *        setDimensions function. Also writes to the newly created
     *        location
     * \param numTimes How many elements to extend the dataset by.
     * \param src Pointer to buffer to write data to file from.
     */
    void extendOnceAndWrite(T *src) {
        extendIR(0, 1);
        int dim = getDimSize();
        this->operator [](dim-1).write(src);
    }
    
    /*!
     * \brief Extends this dataset if it is extendible. I.e. the maximum
     *        dimension of this order was set to H5S_UNLIMITED using the
     *        setDimensions function. Also writes to the newly created
     *        location
     * \param numTimes How many elements to extend the dataset by.
     * \param src Pointer to buffer to write data to file from.
     */
    void extendOnceAndWriteRaw(const void *src) {
        extendIR(0, 1);
        int dim = getDimSize();
        this->operator [](dim-1).writeRaw(src);
    }
    
    /*!
     * \brief operator = Overloaded assignment operator for copying one
     *        dataset to another equivalent to a read from the other then
     *        write.
     * \param rhs Dataset to copy from
     * 
     * Should only be called on the root-level dataset.
     */
    void operator=(CPH5Dataset<T, nDims> &rhs) {
        if (mpDimParent != 0) {
            // Not the root-level dataset
            // Future: proper error. For now just return
            return;
        }
        
        // Test the dimensions...
        bool dimsMatch = true;
        std::vector<int> otherMaxDims = rhs.getMaxDims();
        for (int i = 0; i < nDims; ++i) {
            dimsMatch = dimsMatch && (mMaxDims[i] >= otherMaxDims[i]);
        }
        if (!dimsMatch) {
            // Future: proper error. For now just return
            return;
        }
        
        // Now if necessary make sure this datasets current dimensions
        // match the others dimensions
        std::vector<int> otherDimsI = rhs.getDims();
        std::vector<hsize_t> otherDimsH;
        for (int i = 0; i < otherDimsI.size(); ++i) {
            otherDimsH.push_back(otherDimsI.at(i));
        }
        if (getTotalNumElements() < rhs.getTotalNumElements()) {
            // Call the recursive resize-to function
            resizeToR((hsize_t*)((otherDimsH.data())));
        } else if (getTotalNumElements() > rhs.getTotalNumElements()) {
            // This dataset is already bigger than the other one
            // Future: proper error. For now just return
            return;
        }
        
        // Use mType.getSize instead of sizeof(T) in case T is a compound type.
        int size = rhs.getTotalNumElements()*this->mType.getSize();
        char *buf = new char[size];
        try {
            rhs.readRaw(buf);
            writeRaw(buf);
        } catch (...) {
            delete[] buf;
            throw;
        }

        delete[] buf;
    }
    
    /*!
     * \brief getGroupParent Accessor function for retrieving a pointer
     *        to the parent of this dataset, or NULL if this dataset is
     *        not a child of a group.
     * \return Pointer to parent group, or NULL if parent is not a group.
     */
    CPH5Group *getGroupParent() const {
        return mpGroupParent;
    }
    
    //TODO document
    void setAll(T rhs) {
        int numElements = getTotalNumElements();
        T *pArr = new T[numElements];
        try {
            for (int i = 0; i < numElements; ++i) {
                pArr[i] = rhs;
            }
            write(pArr);
        } catch (...) {
            delete[] pArr;
            throw;
        }
        delete[] pArr;
    }
    
    //TODO document
    CPH5LeafType getLeafType() const override {
        // A non-scalar dataset is never a leaf.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    bool getValIfLeaf(void *p) override {
        // A non-scalar dataset is never a leaf.
        return false;
    }
    
    //TODO document
    bool canIndexInto() const override {
        return true;
    }
    
    //TODO document
    CPH5TreeNode *indexInto(int i) override {
        CPH5TreeNode *ret = 0;
        
        // This is to facilitate the changing of only the
        // last index in the datset, such that it gets latched
        // into memory here and now.
        // Future enhancement: update this to change, instead of
        // append as the default behavior.
        if (nDims == 1) {
            std::vector<int> preInds = mpIOFacility->getIndices();
            std::vector<int> dims = getDims();
            if (preInds.size() == dims.size() && preInds.size() != 0) {
                preInds.pop_back();
                mpIOFacility->setIndices(preInds);
            }
            ret = dynamic_cast<CPH5TreeNode*>(&operator[](i));
            // Force a read to happen here if we have
            // selected all the necessary indices.
            T temp;
            mNextDim.read(&temp);
        } else {
            ret = dynamic_cast<CPH5TreeNode*>(&operator[](i));
        }
        return ret;
    }
    
    //TODO document
    int getIndexableSize() const override {
        return getDims().at(0);
    }
    
    //TODO document
    CPH5LeafType getElementType() const {
        return static_cast<CPH5LeafType>(CPH5TreeNode::IsLeaf<T>::Get);
    }
    
    //TODO document
    int getMemorySizeBelow() const {
        return mpIOFacility->getSizeLowerElements();
    }
    
    //TODO document
    bool readAllBelow(void *p) {
        readRaw(p);
        return true;
    }
    
    //TODO document
    void *getMemoryLocation() const {
        return 0;
    }
    
    //TODO document
    std::vector<std::string> getChildrenNames() const override {
        return std::vector<std::string>();
    }
    
    //TODO document
    CPH5TreeNode *getChildByName(std::string name) const override {
        return 0;
    }
    
    //TODO document
    CPH5Dataset<T, 0> *getScalarRef() {
        return mNextDim.getScalarRef();
    }
    
    
private:
    
    // Friend the orders one above and one below so they can access the private
    // methods. 
    friend class CPH5Dataset<T, nDims+1>;
    friend class CPH5Dataset<T, nDims-1>;
    
    // Disable copy & assignment constructors by making them private
    CPH5Dataset(const CPH5Dataset &other);
    //CPH5Dataset &operator=(const CPH5Dataset &other);
    
    // Also disable the move constructors
    CPH5Dataset(CPH5Dataset &&other);
    CPH5Dataset &operator=(CPH5Dataset &&other);
    
    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the 
     *        type is compound (inherits from CPH5CompType).
     * \param parent Pointer to parent dataset. 
     */
    CPH5Dataset(CPH5Dataset<T, nDims+1> *parent)
        : CPH5GroupMember(""),
          mpGroupParent(0),
          mpDimParent(parent),
          mNextDim(this),
          mpDataSet(0),
          mDimsSet(false),
          mpIOFacility(parent->getIOFacility()),
          mChunksSet(false),
          mDeflateSet(false),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility())
    {
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        
        // THIS MUST BE DONE IN THE CONSTRUCTOR INSTEAD OF THE
        // INITIALIZER LIST. Property lists maintain static ID's
        // under the hood that force us to use the assignment
        // operator instead of the copy constructor.
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    
     /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is compound but a separate type from the template argument
     *        must be used.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5Dataset(CPH5Dataset<T, nDims+1> *parent,
                H5::CompType type)
        : CPH5GroupMember(""),
          mpGroupParent(0),
          mpDimParent(parent),
          mNextDim(this, type),
          mpDataSet(0),
          mDimsSet(false),
          mpIOFacility(parent->getIOFacility()),
          mChunksSet(false),
          mDeflateSet(false),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility(), type)
    {
        // Should only be used if a dataset of non-compound types
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        
        // THIS MUST BE DONE IN THE CONSTRUCTOR INSTEAD OF THE
        // INITIALIZER LIST. Property lists maintain static ID's
        // under the hood that force us to use the assignment
        // operator instead of the copy constructor.
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    
    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is <b>not</b> compound.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5Dataset(CPH5Dataset<T, nDims+1> *parent,
                H5::DataType type)
        : CPH5GroupMember(""),
          mpGroupParent(0),
          mpDimParent(parent),
          mNextDim(this, type),
          mpDataSet(0),
          mDimsSet(false),
          mpIOFacility(parent->getIOFacility()),
          mChunksSet(false),
          mDeflateSet(false),
          CPH5DatasetBase<T, nDims, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility(), type)
    {
        // Should only be used if a dataset of non-compound types
        memset(mDims, 0, nDims*4);
        memset(mMaxDims, 0, nDims*4);
        
        // THIS MUST BE DONE IN THE CONSTRUCTOR INSTEAD OF THE
        // INITIALIZER LIST. Property lists maintain static ID's
        // under the hood that force us to use the assignment
        // operator instead of the copy constructor.
        mPropList = H5::DSetCreatPropList::DEFAULT;
    }
    
    
    /*!
     * \brief Returns pointer to the CPH5IOFacility maintained by this dataset
     *        tree, or 0 if it has not yet been created.
     * \return Pointer to CPH5IOFacility object maintained by this dataset
     *         tree.
     */
    CPH5IOFacility *getIOFacility() {
        if (mpGroupParent != 0) {
            return mpIOFacility;
        } else {
            return mpDimParent->getIOFacility();
        }
    }
    
    
    /*!
     * \brief Inverse-Recursive (IR) function needed to facilitate the
     *        extension of lower-order dimensions in a dataset tree. See the 
     *        public extend function. 
     * \param dimsBelow The number of dimensions below the root-order dimension
     *        that the dataset to extend exists at. 
     * \param numTimes The number of times to extend that dimension.
     * 
     * Recurses upward if not root-order dataset object. Otherwise, increments
     * the local dimension array and extends the dataset in the target HDF5
     * file via the local H5::DataSet object.
     */
    void extendIR(int dimsBelow, int numTimes) {
        if (mpGroupParent != 0) {
            if (!mDimsSet) {
                // Future: proper error. For now just return
                return;
            }
            // Root level
            hsize_t newDims[nDims+1];
            memcpy(newDims, mDims, (nDims+1)*sizeof(hsize_t));
            newDims[dimsBelow] += numTimes;
            
            if (mpDataSet != 0) {
                mpDataSet->extend(newDims);
                memcpy(mDims, newDims, (nDims+1)*sizeof(hsize_t));
            } else {
                //Future: proper error. For now just return.
                return;
            }
            
        } else {
            mpDimParent->extendIR(dimsBelow+1, numTimes);
        }
    }
    
    
    /*!
     * \brief Recursive function for resizing the entire dataset all at once.
     * \param dims Sizes of this objects rank to resize to.
     */
    void resizeToR(hsize_t *dims) {
        int dim = getDimSize();
        if (dims[0] > dim) {
            extend(dims[0] - dim);
        }
        mNextDim.resizeToR(dims+1);
    }
    
    
    /*!
     * \brief Inverse-recursive function to facilitate the getDimSize function
     *        for lower-order dataset dimensions in a dataset tree. See the
     *        public getDimSize function.
     * \param dimsBelow The number of dims below the root-order dimension
     *        to look up in the dimensions array. Incremented by one each time
     *        the function recurses.
     * \return The dimension of the selected dataset through unwind. 
     */
    int getDimSizeIR(int dimsBelow) const {
        if (mpGroupParent != 0) {
            if (!mDimsSet) {
                //Future: proper error. For now just return.
                return 0;
            }
            return mDims[dimsBelow];
        } else {
            return mpDimParent->getDimSizeIR(dimsBelow+1);
        }
    }
    
    
    /*!
     * \brief Inverse-recursive function to facilitate the getMaxDimSize
     *        function for lower-order dataset dimensions in a dataset tree.
     *        See the public getDimSize function.
     * \param dimsBelow The number of dims below the root-order dimension
     *        to look up in the dimensions array. Incremented by one each time
     *        the function recurses.
     * \return The max dimension of the selected dataset through unwind. 
     */
    int getMaxDimSizeIR(int dimsBelow) const {
        if (mpGroupParent != 0) {
            if (!mDimsSet) {
                //Future: proper error. For now just return.
                return 0;
            }
            return mMaxDims[dimsBelow];
        } else {
            return mpDimParent->getMaxDimSizeIR(dimsBelow+1);
        }
    }
    
    
    
    CPH5Group *mpGroupParent;
    CPH5Dataset<T, nDims+1> *mpDimParent;
    CPH5IOFacility *mpIOFacility;
    CPH5Dataset<T, nDims-1> mNextDim;
    hsize_t mDims[nDims+1];
    hsize_t mMaxDims[nDims+1];
    H5::DataSet *mpDataSet;
    H5::DSetCreatPropList mPropList;
    bool mDimsSet;
    bool mChunksSet;
    bool mDeflateSet;
    
    typedef std::vector<CPH5AttributeInterface *> ChildList;
    ChildList mChildren;
};




/*!
 * \brief The CPH5Dataset<T,0> class is a terminal templated
 *        implementation of the CPH5Dataset class that allows
 *        for individual member access. Also known as a 'scalar'
 *        dataset.
 */
template<class T>
class CPH5Dataset<T, 0> :
        public CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>,
        public CPH5GroupMember,
        public CPH5AttributeHolder,
        public CPH5DatasetIdBase
{
public:
    
    
    /*!
     * \brief Public constructor for root-order (also zero) objects that are
     *        instantiated with a non-compound type. This means that the H5::
     *        DataType (usually a PredType) must be passed in. Attempting to
     *        use this constructor with a compound type in the template list
     *        will result in a compiler error.
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     * \param type The HDF5 DataType to use when storing the non-compound 
     *             element.
     */
    CPH5Dataset(CPH5Group *parent,
                std::string name,
                H5::DataType type)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility, type),
          mpDataSet(0)
    {
        parent->registerChild(this);
    }
    
    /*!
     * \brief Public constructor for root-order (also zero) objects that are
     *        instantiated with a compound type. This means that the H5::
     *        CompType passed in will be used instead of the type derived
     *        from the template argument.
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     * \param type The HDF5 CompType to use when storing the compound 
     *             element.
     */
    CPH5Dataset(CPH5Group *parent,
                std::string name,
                H5::CompType type)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility, type),
          mpDataSet(0)
    {
        parent->registerChild(this);
    }
    
    
    /*!
     * \brief Public constructor for root-order (also zero) objects that are
     *        instantiated <i>with a compound datatype</i>. This means that the
     *        template parameter should be a class/struct inherited from
     *        CPH5CompType. This constructur must be used with compound types,
     *        disallowing the explicit passing of an H5::DataType into the 
     *        dataset class - as it will be derived from the given compound
     *        type.
     * \param parent The group to which this dataset belongs.
     * \param name The name of the dataset visible in the HDF5 file.
     */
    CPH5Dataset(CPH5Group *parent,
                std::string name)
        : CPH5GroupMember(name),
          mpGroupParent(parent),
          mpDimParent(0),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(mpIOFacility = new CPH5IOFacility),
          mpDataSet(0)
    {
        parent->registerChild(this);
    }
    
    
    /*!
     * \brief Destructor. Calls closeR and deletes the
     *        CPH5IOFacility member if it has been created.
     */
    virtual ~CPH5Dataset() {
        closeR();
        if (mpGroupParent != 0 && mpIOFacility != 0) {
            delete mpIOFacility;
            mpIOFacility = 0;
        }
    }
    
    
    /*!
     * \brief Recursive open function. Generally called by the parent of this 
     *        dataset object, whether that be a group or another dataset. The R
     *        suffix to the function name means this is a Recursive 
     *        function - calls to openR higher in the tree will recurse down
     *        the tree to this object, and this object calls it's children's 
     *        openR function. 
     * \param create A flag for whether to create the dataset or open it.
     */
    void openR(bool create) {
        if (mpGroupParent == 0)
            return;
        if (create) {
            H5::DataSpace space(0, 0);
                mpDataSet = mpGroupParent->createDataSet(mName, this->mType, space);
        } else {
            mpDataSet = mpGroupParent->openDataSet(mName);
            H5::DataSpace filespace(mpDataSet->getSpace());
            if (filespace.getSimpleExtentNdims() != 0) {
                //Future: proper error. For now just return.
            }
        }
        mpIOFacility->init(mpDataSet, this->mType, 0, 0);
        if (mChildren.size() > 0) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->openR(create);
            }
        }
    }
    
    
    /*!
     * \brief Recursive close function. Calls closeR of all registered children
     *        and then deletes the member dataset if it is created AND this
     *        is a root-order object. 
     */
    void closeR() {
        if (mChildren.size() > 0) {
            for(ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it) {
                (*it)->closeR();
            }
        }
        if (mpDataSet != 0 && mpGroupParent != 0) {
            mpDataSet->close();
            delete mpDataSet;
            mpDataSet = 0;
        }
    }
    
    /*!
     * \brief operator = passes the assignment overload from a T into the base
     *        class implementation since this is a scalar specialization.
     * \param rhs Value to write.
     */
    void operator=(T &rhs) {
        CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::operator=(rhs);
    }
    
    /*!
     * \brief operator = passes the assignment overload from a T into the base
     *        class implementation since this is a scalar specialization.
     * \param rhs Value to write.
     */
    void operator=(T &&rhs) {
        CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::operator=(rhs);
    }
    
    /*!
     * \brief Returns a pointer to the H5::DataSet object maintained by this
     *        CPH5Dataset tree, or 0 if one has not been created yet (the
     *        file has not been opened or created).
     * \return Pointer to H5::DataSet object on which this CPH5Dataset tree
     *         operates, or 0 if it has not been created yet.
     */
    H5::DataSet *getDataSet() const {
        if (mpGroupParent != 0) {
            return mpDataSet;
        } else if (mpDimParent != 0) {
            return mpDimParent->getDataSet();
        } else {
            //Future: proper error. For now just return.
            return 0;
        }
    }
    
    
    /*!
     * \brief Creates an H5::Attribute attached to this dataset in the target
     *        HDF5 file. If the file has not been opened or created, does
     *        nothing and returns 0;
     * \param name The name to give to the attribute visible in the target HDF5
     *        file.
     * \param dataType The H5::DataType to give to the attribute visible in the
     *        target HDF5 file.
     * \param space The H5::DataSpace to use when defining the dimensions of
     *        the attribute in the target HDF5 file.
     * \return A pointer to the newly created attribute, or 0 if one could not 
     *         be created.
     */
    H5::Attribute *createAttribute(std::string name,
                                   H5::DataType dataType,
                                   H5::DataSpace space) {
        if (mpGroupParent != 0) {
            // Root level
            if (mpDataSet != 0) {
                return new H5::Attribute(mpDataSet->createAttribute(name,
                                                                    dataType,
                                                                    space));
            }
        }
        return mpDimParent->createAttribute(name, dataType, space);
    }
    
    
    /*!
     * \brief Opens an H5::Attribute with the given name attached to this
     *        dataset object in the target HDF5 file and returns a pointer to
     *        it, or zero if the attribute could not be opened.
     * \param name Name of attribute to open.
     * \return Pointer to attribute if opened, or 0 if could not be opened.
     */
    H5::Attribute *openAttribute(std::string name) {
        if (mpGroupParent != 0) {
            // Root level
            if (mpDataSet != 0) {
                return new H5::Attribute(mpDataSet->openAttribute(name));
            }
        }
        return mpDimParent->openAttribute(name);
    }
    
    
    /*!
     * \brief Should only be called by CPH5Attribute children of this dataset
     *        object to add themselves to the parent-child tree.
     * \param child Pointer to CPH5Attribute child to register.
     */
    void registerAttribute(CPH5AttributeInterface *child) {
        if (mpGroupParent != 0) {
            // Root level
            mChildren.push_back(child);
        } else {
            return mpDimParent->registerAttribute(child);
        }
    }
    
    
    /*!
     * \brief Should only be called by CPH5Attribute children of the dataset
     *        object to remove itself from the parent-child tree. Usually
     *        during destruction.
     * \param child Pointer to CPH5Attribute child to un-register.
     * 
     * This function may be totally unnecessary as natural object destruction
     * should work just as well.
     */
    void unregisterAttribute(const CPH5AttributeInterface *child) {
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            if ((*it) == child) {
                it = mChildren.erase(it);
            }
        }
        if (mpDimParent != 0) {
            mpDimParent->unregisterAttribute(child);
        }
    }
    
    
    /*!
     * \brief getDims Returns empty vector since this is a scalar dataset.
     * \return Empty vector.
     */
    std::vector<int> getDims() const {
       return std::vector<int>();
    }
    
    /*!
     * \brief getGroupParent Accessor function for retrieving a pointer
     *        to the parent of this dataset, or NULL if this dataset is
     *        not a child of a group.
     * \return Pointer to parent group, or NULL if parent is not a group.
     */
    CPH5Group *getGroupParent() const {
        return mpGroupParent;
    }
    
    
    // All these leafnode functions need to call into the CPH5 base
    
    //TODO document
    CPH5LeafType getLeafType() const override {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getLeafType();
    }
    
    //TODO document
    bool getValIfLeaf(void *p) override {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getValIfLeaf(p);
    }
    
    //TODO document
    bool canIndexInto() const override {
        return false;
    }
    
    //TODO document
    CPH5TreeNode *indexInto(int i) override {
        return 0;
    }
    
    //TODO document
    int getIndexableSize() const override {
        return 0;
    }
    
    //TODO document
    CPH5LeafType getElementType() const {
        // Scalar datasets do not have an element type because they
        // are not an array.
        return LT_IS_NOT_LEAF;
    }
    
    //TODO document
    int getMemorySizeBelow() const {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getTotalMemorySize();
    }
    
    //TODO document
    bool readAllBelow(void *p) {
        this->readRaw(p);
        return true;
    }
    
    //TODO document
    void *getMemoryLocation() const {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getMemoryLocation();
    }
    
    //TODO document
    std::vector<std::string> getChildrenNames() const override {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getChildrenNames();
    }
    
    //TODO document
    CPH5TreeNode *getChildByName(std::string name) const override {
        return CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>::getChildByName(name);
    }
    
    CPH5Dataset<T, 0> *getScalarRef() {
        return this;
    }
    
private:
    
    friend class CPH5Dataset<T, 1>;
    
    // Disable copy & assignment constructors
    CPH5Dataset(const CPH5Dataset &other);
    CPH5Dataset &operator=(const CPH5Dataset &other);
    
    
    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the 
     *        type is compound (inherits from CPH5CompType).
     * \param parent Pointer to parent dataset. 
     */
    CPH5Dataset(CPH5Dataset<T, 1> *parent)
        : CPH5GroupMember(""),
          mpDimParent(parent),
          mpGroupParent(0),
          mpDataSet(0),
          mpIOFacility(parent->getIOFacility()),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility())
    {
        this->mType = parent->mType;
    }
    
    
    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is <b>not</b> compound.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5Dataset(CPH5Dataset<T, 1> *parent,
                H5::DataType type)
        : CPH5GroupMember(""),
          mpDimParent(parent),
          mpGroupParent(0),
          mpDataSet(0),
          mpIOFacility(parent->getIOFacility()),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility(), type)
    {} // NOOP
    
    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is compound, but a different type than the one derived
     *        from the template parameter is desired to be used.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5Dataset(CPH5Dataset<T, 1> *parent,
                H5::CompType type)
        : CPH5GroupMember(""),
          mpDimParent(parent),
          mpGroupParent(0),
          mpDataSet(0),
          mpIOFacility(parent->getIOFacility()),
          CPH5DatasetBase<T, 0, IsDerivedFrom<T, CPH5CompType>::Is>(parent->getIOFacility(), type)
    {} // NOOP
    
    
    /*!
     * \brief Recursive function for resizing the entire dataset all at once.
     * \param dims Sizes of this objects rank to resize to.
     */
    void resizeToR(hsize_t *dims) {
        // This should never be called
        //Future: proper error. For now just return.
        return;
    }
    
    H5::DataSet *mpDataSet;
    CPH5Group *mpGroupParent;
    CPH5Dataset<T, 1> *mpDimParent;
    CPH5IOFacility *mpIOFacility;
    
    typedef std::vector<CPH5AttributeInterface*> ChildList;
    ChildList mChildren;
};


/*!
 * \cond
 * 
 * This can be ignored in the documentation since it is only a terminal
 * implementation to prevent infinite recursion in the compiler.
 */
template<class T>
class CPH5Dataset<T, CPH_5_MAX_DIMS+1>
        : public CPH5DatasetBase<T, CPH_5_MAX_DIMS+1, IS_NOT_DERIVED>
{
public:
    
    H5::DataSet *getDataSet() const {
        return 0;
    }
    void addIndex(int) {} // NOOP
    void readIR(T *) {} // NOOP
    void writeIR (const T *) {} // NOOP
    CPH5IOFacility *getIOFacility() {
        return 0;
    }
    H5::Attribute *createAttribute(std::string name,
                                   H5::DataType dataType,
                                   H5::DataSpace space) {
        return 0;
    }
    H5::Attribute *openAttribute(std::string name) {
        return 0;
    }
    void registerAttribute(CPH5AttributeInterface *) {} // NOOP
    void unregisterAttribute(const CPH5AttributeInterface *) {} // NOOP
    void extendIR(int, int) {} // NOOP
    int getDimSizeIR(int) {return 0;} // NOOP
    int getMaxDimSizeIR(int) {return 0;} // NOOP
    
    // These may or may not be necessary.
    CPH5TreeNode::CPH5LeafType getLeafType() const { return CPH5TreeNode::LT_IS_NOT_LEAF; }
    bool getValIfLeaf(void *p) { return false; }
    bool canIndexInto() const { return false; }
    CPH5TreeNode *indexInto(int i) { return 0; }
    int getIndexableSize() const { return 0; }
    void *getMemoryLocation() const { return 0; }
    std::vector<std::string> getChildrenNames() const { return std::vector<std::string>(); }
    CPH5TreeNode *getChildByName(std::string name) const { return 0; }
    
private:
    CPH5Dataset()
        : CPH5DatasetBase<T, CPH_5_MAX_DIMS+1, IS_NOT_DERIVED>(0)
    {} // NOOP
    CPH5Dataset(const CPH5Dataset &other);
};
/*!
 * \endcond
 */


#endif // CPH5DATASET_H
