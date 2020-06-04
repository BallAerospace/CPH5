////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5VARLENSTR_H
#define CPH5VARLENSTR_H

#include "cph5utilities.h"
#include "cph5dataset.h"
#include "cph5group.h"
#include <string>


/**
 * \brief The CPH5StrIOFacility class is a convenience object
 *        for maintaining hyperslab selections through layers
 *        of a multidimensional variable length strings arrays
 */
class CPH5StrIOFacility
{
public:

    /**
     * \brief CPH5CPH5StrIOFacility Default constructor.
     */
    CPH5StrIOFacility() :
            mpDataSet(nullptr),
            mType(H5::StrType(0, H5T_VARIABLE)),
            numDims(-1),
            mNumElem(1)
    {
    }

    /**
     * \brief Initializes the CPH5StrIOFacility with the necessary parameters to begin
     *        hyperslab selection.
     * \param pDataSet Pointer to H5::DataSet object to use to read and write.
     * \param nDims The number of dimensions of the dataset.
     * \param maxDims The maximums of each dimension of the dataset.
     *        H5S_UNLIMITED is used to identify an unlimited dimension.
     */
    void init(H5::DataSet *pDataSet,
              int nDims,
              hsize_t *maxDims)
    {
        mpDataSet = pDataSet;
        numDims = nDims;

        // check to make sure the value is not negative
        if (nDims < 0)
        {
            //set it to negative one if any negative value comes in
            numDims = -1;
        }

        mMaxDims.clear();
        mIndices.clear();
        for (int i = 0; i < nDims; ++i)
        {
            mMaxDims.push_back(static_cast<int>(maxDims[i]));
        }
    }

    /**
     * \brief Used when an index is selected for one of the dataset dimensions.
     *        It is stored into the IOFacility and used later upon a call to
     *        read or write.
     * \param ind Index of dimension.
     */
    void addIndex(int ind)
    {
        if (numDims == -1)
        {
            // BIG PROBLEM, UNINITIALIZED
            return;
        }
        mIndices.push_back(ind);
        if (mIndices.size() > static_cast<std::size_t>(numDims))
        {
            // BIG PROBLEM, TOO MANY INDICES
        }
    }

    /**
     * \brief Write data from the given std::string to the target HDF5 file through
     *        the H5::DataSet object given to this object from init().
     *
     * \param src string with data.
     */
    void write(const std::vector<std::string> &src)
    {
        if (mpDataSet == nullptr)
        {
            return;
        }
        setupSpaces();

        //check to make sure the size of the vector matches the number of elements
        //selected to write
        if (mNumElem != src.size())
        {
            std::string errMsg;
            errMsg.append("Number of elements to write does not match number of ");
            errMsg.append("elements in selection");
            throw std::runtime_error(errMsg);
        }

        std::vector<const char *> arr_c_str;
        for (std::size_t ii = 0; ii < src.size(); ++ii)
        {
            arr_c_str.emplace_back(src[ii].c_str());
        }

        mpDataSet->write(arr_c_str.data(), mType, mMemspace, mFilespace);

    }

    /**
     * \brief Read data from the target HDF5 file through the given H5::DataSet
     *        object into the buffer. It is assumed that the buffer is large
     *        enough to store all the data existing below this point in the
     *        data tree.
     * \param dst Buffer to store data into.
     */
    void read(std::vector<std::string> &dst)
    {
        if (mpDataSet == nullptr)
        {
            return;
        }
        setupSpaces();

        //Allocate space for reading back the string pointers
        char **cReadVal;
        cReadVal = new char*[mNumElem + 1];

        //read the data
        mpDataSet->read(cReadVal, mType, mMemspace, mFilespace);

        //create a vector with the data read in
        for (hsize_t i = 0; i < mNumElem; ++i)
        {
            std::string readVal;
            //make sure the value read in is not null
            if (cReadVal[i] != nullptr)
            {
                //add the string to the vector
                readVal.assign(cReadVal[i]);
                dst.emplace_back(readVal);
            }
            else
            {
                //assign empty if null
                readVal.assign("");
            }
        }

        //reclaim the memory allocated by the library for the variable length
        //arrays
        mpDataSet->vlenReclaim(cReadVal, mType, mMemspace);

        //delete the memory allocated for the memory
        delete[] cReadVal;

    }

    /**
     * \brief Calculates the total of selected elements that are currently
     *        selected in the dataset.
     * \return Number of selected elements.
     */
    hsize_t getNumLowerElements()
    {
        setupSpaces();
        return mFilespace.getSelectNpoints();
    }

    /**
     * \brief getIndices Returns the current list of indices
     * \return A copy of the list of indices
     */
    std::vector<int> getIndices() const
    {
        return mIndices;
    }

private:



    /**
     * \brief This function is used to set up the dataspaces necessary for a
     *        hyperslab selection with the indexes added to this IOFacility
     *        with addIndex(). This should be done before any reads or writes
     *        to the H5::DataSet object given during init().
     */
    void setupSpaces()
    {
        if (numDims == -1)
        {
            // BIG PROBLEM
            return;
        }

        //initalize to zero
        memset(mOffsets, 0, CPH_5_MAX_DIMS * 4);
        memset(mNumSteps, 0, CPH_5_MAX_DIMS * 4);

        //Initialize all to false
        std::fill(mIncrementOffset, mIncrementOffset + CPH_5_MAX_DIMS, false);

        mNumElem = 1;

        for (std::size_t i = 0; i < mIndices.size(); ++i)
        {
            mOffsets[i] = mIndices[i];
        }

        for (std::size_t i = 0; i < static_cast<std::size_t>(numDims); ++i)
        {
            if (i < mIndices.size())
            {
                mExtents[i] = 1;
            }
            else
            {
                mExtents[i] = mMaxDims[i];
                mIncrementOffset[i] = true;
            }

            //Multiply all the sizes so we can get the
            //total number of elements read
            mNumElem *= mExtents[i];
        }

        hsize_t readSize[CPH_5_MAX_DIMS] = { mNumElem };

        if (mpDataSet != 0)
        {
            mFilespace = mpDataSet->getSpace();
            mMemspace = H5::DataSpace();
            if (numDims != 0)
            {
                mFilespace.selectHyperslab(H5S_SELECT_SET, mExtents, mOffsets);
                mMemspace = H5::DataSpace(1, readSize, NULL);
            }
        }
    }

    H5::DataSet *mpDataSet;
    H5::DataType mType;

    hsize_t mOffsets[CPH_5_MAX_DIMS];
    hsize_t mExtents[CPH_5_MAX_DIMS];
    hsize_t mNumSteps[CPH_5_MAX_DIMS];
    bool mIncrementOffset[CPH_5_MAX_DIMS];

    int numDims;
    std::vector<int> mMaxDims;
    std::vector<int> mIndices;

    hsize_t mNumElem;
    H5::DataSpace mMemspace;
    H5::DataSpace mFilespace;
};

/**
 * \brief The CPH5VarLenStrBase class is the base class for all variable
 *        length strings in HDF5. It's specialization
 *        is dependent on whether whether the dataset object is order 0
 *        (scalar) or not.
 *
 * If this particular implementation is used instead of one of the
 * specialized versions, it means that this particular object is for
 * a non-0 order (more than just a single element). Implicitly, the
 * H5VarLengStrBase<0> will be implemented for the order 0
 * object. The first template parameter must be a constant integer for
 * the number of dimensions of the particular dataset.
 *
 * The reason the for the specializations is that the read
 * and write functionality must be different for order-0 objects
 * versus 1+ order objects that must provide the assignment operator.
 * Thus we have two options for CPH5VarLenStrBase: order 1+, order 0
 */
template<const int nDims>
class CPH5VarLenStrBase
{
    // order 1+
public:

    /**
     * \brief CPH5VarLenStrBase constructor, accepts CPH5StrIOFacility pointer (as
     *        do all H5VarLenStrBases). This is a non-scalar specialization.
     * \param pioFacility Pointer to CPH5StrIOFacility to use for reads/writes.
     */
    CPH5VarLenStrBase(CPH5StrIOFacility *pioFacility)
    {
        mpIOFacility = pioFacility;
        mType = H5::StrType(0, H5T_VARIABLE);
    }

    /**
     * \brief Reads data at this dimension level into the given buffer from the
     *        target HDF5 file.
     * \param buf T pointer to buffer that must be large enough to accept the
     *        data.
     */
    void read(std::vector<std::string> &buf)
    {
        if (mpIOFacility != 0)
        {
            mpIOFacility->read(buf);
        }
    }

    /**
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file.
     * \param src T pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void write(const std::vector<std::string> &src)
    {
        mpIOFacility->write(src);
    }

protected:

    H5::DataType mType;

private:

    CPH5StrIOFacility *mpIOFacility;
};

/**
 * \brief The CPH5VarLenStrBase<0> class is a specialization
 *        of the CPH5VarLenStrBase template class, specific for
 *        an order 0 object.
 */
template<>
class CPH5VarLenStrBase<0>
{
    // ORDER 0
public:

    /**
     * \brief CPH5VarLenStrBase constructor, accepts CPH5StrIOFacility pointer (as
     *        do all H5VarLenStrBases). This is a scalar specialization.
     * \param pioFacility Pointer to CPH5IOFacility to use for reads/writes.
     */
    CPH5VarLenStrBase(CPH5StrIOFacility *pioFacility)
    {
        mpIOFacility = pioFacility;
        mType = H5::StrType(0, H5T_VARIABLE);
    }

    /**
     * \brief operator T Cast operator to the type for which this class is
     *        templated so that it can be read using basic assignment. I.e.
     *        T a = *this;
     */
    operator std::string()
    {
        std::string retVal;
        read(retVal);
        return retVal;
    }

    /**
     * \brief Reads data at this dimension level into the given buffer from the
     *        target HDF5 file.
     * \param buf T pointer to buffer that must be large enough to accept the
     *        data.
     */
    void read(std::string &buf)
    {
        //clear buffer
        mSBuf.clear();

        //read into buffer
        mpIOFacility->read(mSBuf);
        buf = mSBuf[0];

    }

    /**
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file.
     * \param src T pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void write(const std::string &src)
    {
        //clear buffer
        mSBuf.clear();
        mSBuf.emplace_back(src);
        mpIOFacility->write(mSBuf);
    }

    /**
     * \brief operator = Overloaded assignment operator to allow assignment of
     *        this element equivalent to a write, since this is a scalar-order
     *        dataset.
     * \param rhs Value to write.
     */
    void operator=(std::string &rhs)
    {
        write(rhs);
    }

    int getTotalMemorySize() const {
        return static_cast<int>(mSBuf[0].size());
    }

    // Note that these are NOT virtual - this is by design.

    //TODO document
    CPH5TreeNode::CPH5LeafType getLeafType() const
    {
        return static_cast<CPH5TreeNode::CPH5LeafType>(CPH5TreeNode::IsLeaf<std::string>::Get);
    }

    //TODO document
    bool getValIfLeaf(void * /*p*/)
    {
        if (getLeafType() != CPH5TreeNode::LT_IS_NOT_LEAF)
        {
            //read(p);
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
        return static_cast<int>(mSBuf[0].size());
    }

    //TODO document
    bool readAllBelow(void *p) {
        return getValIfLeaf(p);
    }

    //TODO document
    void *getMemoryLocation() const {
        return nullptr;
    }

    //TODO document
    std::vector<std::string> getChildrenNames() const
    {
        // The template argument is not inherited from CPH5CompType, so there
        // is no children.
        return std::vector<std::string>();
    }

    //TODO document
    CPH5TreeNode *getChildByName(std::string /*name*/) const
                                 {
        // The template argument is not inherited from CPH5CompType, so there
        // is no children.
        return 0;
    }
protected:
    /**
     * \brief Reads data at this dimension level into the given buffer from the
     *        target HDF5 file.
     * \param buf T pointer to buffer that must be large enough to accept the
     *        data.
     */
    void read(std::vector<std::string> &buf)
    {
        if (mpIOFacility != 0)
        {
            mpIOFacility->read(buf);
        }
    }

    /**
     * \brief Writes data at this dimensions level from the given buffer to the
     *        target HDF5 file.
     * \param src T pointer to buffer to source data from. Must be large enough
     *        to read all necessary data from.
     */
    void write(const std::vector<std::string> &src)
    {
        mpIOFacility->write(src);
    }
protected:

    H5::DataType mType;

private:

    CPH5StrIOFacility *mpIOFacility;

    std::vector<std::string> mSBuf;
};

/*!
 * \brief The CPH5VarLenStr class defines a multi-dimensional dataset and allows
 *        access to individual elements or array subsets.
 *
 * The first template parameter is the type that the object should be treated
 * as an array of in C++ code. The second parameter must be a constant integer
 * for the number of dimensions in the array - ie. 3 would mean a 3 dimensional
 * array, 2 would be a matrix, 1 a list and 0 a single element.
 *
 * The CPH5VarLenStr class is a recursive template - meaning any implementation
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
template<const int nDims>
class CPH5VarLenStr:
                   public CPH5GroupMember,
                   public CPH5AttributeHolder,
                   // SFINAE
                   public CPH5VarLenStrBase<nDims>,
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
    CPH5VarLenStr(CPH5Group *parent,
                std::string name)
    :
            CPH5GroupMember(name),
            CPH5VarLenStrBase<nDims>(mpIOFacility = new CPH5StrIOFacility),
            mpGroupParent(parent),
            mpDimParent(0),
            mNextDim(this),
            mpDataSet(0),
            mDimsSet(false),
            mChunksSet(false),
            mDeflateSet(false)
    {
        memset(mDims, 0, nDims * 4);
        memset(mMaxDims, 0, nDims * 4);
        parent->registerChild(this);

        mPropList = H5::DSetCreatPropList::DEFAULT;
    }

    /*!
     * \brief Destructor. Calls closeR and deletes the
     *        CPH5IOFacility member if it has been created.
     */
    virtual ~CPH5VarLenStr()
    {
        closeR();
        if (mpGroupParent != 0 && mpIOFacility != 0)
        {
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
    void openR(bool create)
    {
        if (!mDimsSet && create)
        {
            // Future: proper error. For now just return
            return;
        }
        if (mpGroupParent == 0)
            return;
        if (create)
        {
            H5::DataSpace space(nDims, mDims, mMaxDims);
            if (mChunksSet)
            {
                mpDataSet = mpGroupParent->createDataSet(mName,
                        this->mType,
                        space,
                        mPropList);
            }
            else
            {
                mpDataSet = mpGroupParent->createDataSet(mName, this->mType, space);
            }
        }
        else
        {
            mpDataSet = mpGroupParent->openDataSet(mName);
            H5::DataSpace filespace(mpDataSet->getSpace());
            if (filespace.getSimpleExtentNdims() != nDims)
            {
                // Future: proper error. For now just return
                return;
            }
            filespace.getSimpleExtentDims(mDims, mMaxDims);
            mDimsSet = true;
        }
        if (mChildren.size() > 0)
        {
            for (ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it)
            {
                (*it)->openR(create);
            }
        }
    }

    /*!
     * \brief Recursive close function. Calls closeR of all registered children
     *        and then deletes the member dataset if it is created AND this
     *        is a root-order object.
     */
    void closeR()
    {
        if (mChildren.size() > 0)
        {
            for (ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it)
            {
                (*it)->closeR();
            }
        }
        if (mpDataSet != 0 && mpGroupParent != 0)
        {
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
     *         the return will be a reference to a CPH5VarLenStr object with the
     *         same type but with order 1 - i.e. a row in the 2D array.
     */
    CPH5VarLenStr<nDims - 1> &operator[](int ind)
    {
        if (mpGroupParent != 0)
        {
            mpIOFacility->init(mpDataSet,
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
    int getDimSize() const
    {
        return getDimSizeIR(0);
    }

    /*!
     * \brief Returns the maxmium size of this dimension, as either set by the
     *        user with setDimensions (if file is being created) or as read
     *        from the target HDF5 file if opened.
     * \return An integer value for the max size of the dimension.
     */
    int getMaxDimSize() const
    {
        return getMaxDimSizeIR(0);
    }

    /*!
     * \brief getDims Returns vector of dimensions for this dataset.
     * \return Vector of dimensions for this dataset.
     */
    std::vector<int> getDims() const
    {
        std::vector<int> ret;
        for (int i = 0; i < nDims; ++i)
        {
            ret.push_back(getDimSizeIR(i));
        }
        return ret;
    }

    /*!
     * \brief getMaxDims Returns vector of maximum dimensions for this dataset.
     * \return Vector of maximum dimensions for this dataset.
     */
    std::vector<int> getMaxDims() const
    {
        std::vector<int> ret;
        for (int i = 0; i < nDims; ++i)
        {
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
                       hsize_t maxDims[nDims])
    {
        memcpy(mDims, dims, nDims * sizeof(hsize_t));
        memcpy(mMaxDims, maxDims, nDims * sizeof(hsize_t));
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
    void setChunkSize(hsize_t chunkDims[nDims])
    {
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
    void setDeflateLevel(int level)
    {
        mPropList.setDeflate(level);
        mDeflateSet = true;
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
    void write(const std::vector<std::string> &src)
    {
        // Can be used at every level
        if (mpGroupParent != 0)
        {
            // Root level
            mpIOFacility->init(mpDataSet,
                    nDims,
                    mDims);
        }
        CPH5VarLenStrBase<nDims>::write(src);
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
    void read(std::vector<std::string> &dst)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            mpIOFacility->init(mpDataSet,
                    nDims,
                    mDims);
        }
        CPH5VarLenStrBase<nDims>::read(dst);
    }

    /*!
     * \brief Returns the total number of elements currently allocated in the
     *        target HDF5 file (whether it has actually been written or not)
     *        below this point in the dataset tree.
     * \return Number of elements with the specified type (from the template)
     *         that exist below this point in the dataset tree.
     */
    int getTotalNumElements() const
    {
        std::vector<int> dims = getDims();
        int ret = dims[0];
        for (int i = 1; i < nDims; ++i)
        {
            ret = ret * dims[i];
        }
        return ret;
    }

    /*!
     * \brief Returns a pointer to the H5::DataSet object maintained by this
     *        CPH5VarLenStr tree, or 0 if one has not been created yet (the
     *        file has not been opened or created).
     * \return Pointer to H5::DataSet object on which this CPH5VarLenStr tree
     *         operates, or 0 if it has not been created yet.
     */
    H5::DataSet *getDataSet() const
    {
        if (mpGroupParent != 0)
        {
            return mpDataSet;
        }
        else if (mpDimParent != 0)
        {
            return mpDimParent->getDataSet();
        }
        else
        {
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
                                   H5::DataSpace space)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            if (mpDataSet != 0)
            {
                return new H5::Attribute(mpDataSet->createAttribute(name,
                        dataType,
                        space));
            }
            else
            {
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
    H5::Attribute *openAttribute(std::string name)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            if (mpDataSet != 0)
            {
                return new H5::Attribute(mpDataSet->openAttribute(name));
            }
            else
            {
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
    void registerAttribute(CPH5AttributeInterface *child)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            mChildren.push_back(child);
        }
        else
        {
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
    void unregisterAttribute(const CPH5AttributeInterface *child)
    {
        for (ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it)
        {
            if ((*it) == child)
            {
                it = mChildren.erase(it);
            }
        }
        if (mpDimParent != 0)
        {
            mpDimParent->unregisterAttribute(child);
        }
    }

    /*!
     * \brief Extends this dataset if it is extendible. I.e. the maximum
     *        dimension of this order was set to H5S_UNLIMITED using the
     *        setDimensions function.
     * \param numTimes How many elements to extend the dataset by.
     */
    void extend(int numTimes)
    {
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
    void extendOnceAndWrite(std::vector<std::string> &src)
    {
        extendIR(0, 1);
        int dim = getDimSize();
        this->operator [](dim - 1).write(src);
    }

    /*!
     * \brief getGroupParent Accessor function for retrieving a pointer
     *        to the parent of this dataset, or NULL if this dataset is
     *        not a child of a group.
     * \return Pointer to parent group, or NULL if parent is not a group.
     */
    CPH5Group *getGroupParent() const
    {
        return mpGroupParent;
    }

    //TODO document
    CPH5LeafType getLeafType() const override
    {
        // A non-scalar dataset is never a leaf.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }

    //TODO document
    bool getValIfLeaf(void * /*p*/) override
    {
        // A non-scalar dataset is never a leaf.
        return false;
    }

    //TODO document
    bool canIndexInto() const override
    {
        return true;
    }

    //TODO document
    CPH5TreeNode *indexInto(int i) override
    {
        return dynamic_cast<CPH5TreeNode*>(&operator[](i));
    }

    //TODO document
    int getIndexableSize() const override
    {
        return getDims().at(0);
    }

    //TODO document
    CPH5LeafType getElementType() const {
        return static_cast<CPH5LeafType>(CPH5TreeNode::IsLeaf<std::string>::Get);
    }

    //TODO document
    int getMemorySizeBelow() const {
        return static_cast<int>(mpIOFacility->getNumLowerElements());
    }

    //TODO document
    bool readAllBelow(void * /*p*/) {
        //read(p);
        return true;
    }

    //TODO document
    void *getMemoryLocation() const {
        return 0;
    }

    //TODO document
    std::vector<std::string> getChildrenNames() const override
    {
        return std::vector<std::string>();
    }

    //TODO document
    CPH5TreeNode *getChildByName(std::string /*name*/) const override
    {
        return 0;
    }

    //TODO document
    CPH5VarLenStr<0> *getScalarRef()
    {
        return mNextDim.getScalarRef();
    }

private:

    // Friend the orders one above and one below so they can access the private
    // methods.
    friend class CPH5VarLenStr<nDims + 1> ;
    friend class CPH5VarLenStr<nDims - 1> ;

    // Disable copy & assignment constructors by making them private
    CPH5VarLenStr(const CPH5VarLenStr &other);
    CPH5VarLenStr &operator=(const CPH5VarLenStr &other);

    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is <b>not</b> compound.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5VarLenStr(CPH5VarLenStr<nDims + 1> *parent) :
            CPH5GroupMember(""),
            mpGroupParent(0),
            mpDimParent(parent),
            mNextDim(this),
            mpDataSet(0),
            mDimsSet(false),
            mpIOFacility(parent->getIOFacility()),
            mChunksSet(false),
            mDeflateSet(false),
            CPH5VarLenStrBase<nDims>(parent->getIOFacility())
    {
        // Should only be used if a dataset of non-compound types
        memset(mDims, 0, nDims * 4);
        memset(mMaxDims, 0, nDims * 4);

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
    CPH5StrIOFacility *getIOFacility()
    {
        if (mpGroupParent != 0)
        {
            return mpIOFacility;
        }
        else
        {
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
    void extendIR(int dimsBelow, int numTimes)
    {
        if (mpGroupParent != 0)
        {
            if (!mDimsSet)
            {
                // Future: proper error. For now just return
                return;
            }
            // Root level
            hsize_t newDims[nDims + 1];
            memcpy(newDims, mDims, (nDims + 1) * sizeof(hsize_t));
            newDims[dimsBelow] += numTimes;

            if (mpDataSet != 0)
            {
                mpDataSet->extend(newDims);
                memcpy(mDims, newDims, (nDims + 1) * sizeof(hsize_t));
            }
            else
            {
                //Future: proper error. For now just return.
                return;
            }

        }
        else
        {
            mpDimParent->extendIR(dimsBelow + 1, numTimes);
        }
    }

    /*!
     * \brief Recursive function for resizing the entire dataset all at once.
     * \param dims Sizes of this objects rank to resize to.
     */
    void resizeToR(hsize_t *dims)
    {
        int dim = getDimSize();
        if (dims[0] > dim)
        {
            extend(dims[0] - dim);
        }
        mNextDim.resizeToR(dims + 1);
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
    int getDimSizeIR(int dimsBelow) const
                     {
        if (mpGroupParent != 0)
        {
            if (!mDimsSet)
            {
                //Future: proper error. For now just return.
                return 0;
            }
            return static_cast<int>(mDims[dimsBelow]);
        }
        else
        {
            return mpDimParent->getDimSizeIR(dimsBelow + 1);
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
    int getMaxDimSizeIR(int dimsBelow) const
                        {
        if (mpGroupParent != 0)
        {
            if (!mDimsSet)
            {
                //Future: proper error. For now just return.
                return 0;
            }
            return mMaxDims[dimsBelow];
        }
        else
        {
            return mpDimParent->getMaxDimSizeIR(dimsBelow + 1);
        }
    }

    CPH5Group *mpGroupParent;
    CPH5VarLenStr<nDims + 1> *mpDimParent;
    CPH5StrIOFacility *mpIOFacility;
    CPH5VarLenStr<nDims - 1> mNextDim;
    hsize_t mDims[nDims + 1];
    hsize_t mMaxDims[nDims + 1];
    H5::DataSet *mpDataSet;
    H5::DSetCreatPropList mPropList;
    bool mDimsSet;
    bool mChunksSet;
    bool mDeflateSet;

    typedef std::vector<CPH5AttributeInterface *> ChildList;
    ChildList mChildren;
};

/*!
 * \brief The CPH5VarLenStr<T,0> class is a terminal templated
 *        implementation of the CPH5VarLenStr class that allows
 *        for individual member access. Also known as a 'scalar'
 *        dataset.
 */
template<>
class CPH5VarLenStr<0> :
                       public CPH5VarLenStrBase<0>,
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
    CPH5VarLenStr(CPH5Group *parent,
                std::string name)
    :
            CPH5VarLenStrBase<0>(mpIOFacility = new CPH5StrIOFacility),
            CPH5GroupMember(name),
            mpDimParent(0),
            mpGroupParent(parent),
            mpDataSet(0)
    {
        parent->registerChild(this);
    }

    /*!
     * \brief Destructor. Calls closeR and deletes the
     *        CPH5IOFacility member if it has been created.
     */
    virtual ~CPH5VarLenStr()
    {
        closeR();
        if (mpGroupParent != 0 && mpIOFacility != 0)
        {
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
    void openR(bool create)
    {
        if (mpGroupParent == 0)
            return;
        if (create)
        {
            H5::DataSpace space(0, 0);
            mpDataSet = mpGroupParent->createDataSet(mName, this->mType, space);
        }
        else
        {
            mpDataSet = mpGroupParent->openDataSet(mName);
            H5::DataSpace filespace(mpDataSet->getSpace());
            if (filespace.getSimpleExtentNdims() != 0)
            {
                //Future: proper error. For now just return.
            }
        }
        mpIOFacility->init(mpDataSet, 0, 0);
        if (mChildren.size() > 0)
        {
            for (ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it)
            {
                (*it)->openR(create);
            }
        }
    }

    /*!
     * \brief Recursive close function. Calls closeR of all registered children
     *        and then deletes the member dataset if it is created AND this
     *        is a root-order object.
     */
    void closeR()
    {
        if (mChildren.size() > 0)
        {
            for (ChildList::iterator it = mChildren.begin();
                    it != mChildren.end();
                    ++it)
            {
                (*it)->closeR();
            }
        }
        if (mpDataSet != 0 && mpGroupParent != 0)
        {
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
    void operator=(std::string &rhs)
    {
        CPH5VarLenStrBase<0>::operator=(rhs);
    }

    /*!
     * \brief operator = passes the assignment overload from a T into the base
     *        class implementation since this is a scalar specialization.
     * \param rhs Value to write.
     */
    void operator=(std::string &&rhs)
    {
        CPH5VarLenStrBase<0>::operator=(rhs);
    }

    /*!
     * \brief Returns a pointer to the H5::DataSet object maintained by this
     *        CPH5VarLenStr tree, or 0 if one has not been created yet (the
     *        file has not been opened or created).
     * \return Pointer to H5::DataSet object on which this CPH5VarLenStr tree
     *         operates, or 0 if it has not been created yet.
     */
    H5::DataSet *getDataSet() const
    {
        if (mpGroupParent != 0)
        {
            return mpDataSet;
        }
        else if (mpDimParent != 0)
        {
            return mpDimParent->getDataSet();
        }
        else
        {
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
                                   H5::DataSpace space)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            if (mpDataSet != 0)
            {
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
    H5::Attribute *openAttribute(std::string name)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            if (mpDataSet != 0)
            {
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
    void registerAttribute(CPH5AttributeInterface *child)
    {
        if (mpGroupParent != 0)
        {
            // Root level
            mChildren.push_back(child);
        }
        else
        {
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
    void unregisterAttribute(const CPH5AttributeInterface *child)
    {
        for (ChildList::iterator it = mChildren.begin();
                it != mChildren.end();
                ++it)
        {
            if ((*it) == child)
            {
                it = mChildren.erase(it);
            }
        }
        if (mpDimParent != 0)
        {
            mpDimParent->unregisterAttribute(child);
        }
    }

    /*!
     * \brief getDims Returns empty vector since this is a scalar dataset.
     * \return Empty vector.
     */
    std::vector<int> getDims() const
    {
        return std::vector<int>();
    }

    /*!
     * \brief getGroupParent Accessor function for retrieving a pointer
     *        to the parent of this dataset, or NULL if this dataset is
     *        not a child of a group.
     * \return Pointer to parent group, or NULL if parent is not a group.
     */
    CPH5Group *getGroupParent() const
    {
        return mpGroupParent;
    }

    // All these leafnode functions need to call into the CPH5 base

    //TODO document
    CPH5LeafType getLeafType() const override
    {
        return CPH5VarLenStrBase<0>::getLeafType();
    }

    //TODO document
    bool getValIfLeaf(void *p) override
    {
        return CPH5VarLenStrBase<0>::getValIfLeaf(p);
    }

    //TODO document
    bool canIndexInto() const override
    {
        return false;
    }

    //TODO document
    CPH5TreeNode *indexInto(int /*i*/) override
    {
        return 0;
    }

    //TODO document
    int getIndexableSize() const override
    {
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
        return getTotalMemorySize();
    }

    //TODO document
    bool readAllBelow(void * /*p*/) {
        //read(p);
        return true;
    }

    //TODO document
    void *getMemoryLocation() const {
        return CPH5VarLenStrBase<0>::getMemoryLocation();
    }


    //TODO document
    std::vector<std::string> getChildrenNames() const override
    {
        return CPH5VarLenStrBase<0>::getChildrenNames();
    }

    //TODO document
    CPH5TreeNode *getChildByName(std::string name) const override
    {
        return CPH5VarLenStrBase<0>::getChildByName(name);
    }

    CPH5VarLenStr<0> *getScalarRef()
    {
        return this;
    }

private:

    friend class CPH5VarLenStr<1> ;

    // Disable copy & assignment constructors
    CPH5VarLenStr(const CPH5VarLenStr &other);
    CPH5VarLenStr &operator=(const CPH5VarLenStr &other);

    /*!
     * \brief Private constructor that should only be used by a higher-order
     *        dataset creating this as it's lower-order child, and if the
     *        type is <b>not</b> compound.
     * \param parent Pointer to parent dataset.
     * \param type H5::DataType to use in the target file.
     */
    CPH5VarLenStr(CPH5VarLenStr<1> *parent)
    :
            CPH5VarLenStrBase<0>(parent->getIOFacility()),
            CPH5GroupMember(""),
            mpDimParent(parent),
            mpGroupParent(0),
            mpDataSet(0),
            mpIOFacility(parent->getIOFacility())
    {
    } // NOOP

    /*!
     * \brief Recursive function for resizing the entire dataset all at once.
     * \param dims Sizes of this objects rank to resize to.
     */
    void resizeToR(hsize_t * /*dims*/)
    {
        // This should never be called
        //Future: proper error. For now just return.
        return;
    }

    CPH5VarLenStr<1> *mpDimParent;
    CPH5Group *mpGroupParent;
    H5::DataSet *mpDataSet;
    CPH5StrIOFacility *mpIOFacility;

    typedef std::vector<CPH5AttributeInterface*> ChildList;
    ChildList mChildren;
};

/*!
 * \cond
 *
 * This can be ignored in the documentation since it is only a terminal
 * implementation to prevent infinite recursion in the compiler.
 */
template<>
class CPH5VarLenStr<CPH_5_MAX_DIMS + 1>
: public CPH5VarLenStrBase<CPH_5_MAX_DIMS + 1>
{
public:

    H5::DataSet *getDataSet() const
    {
        return 0;
    }
    void addIndex(int)
    {
    } // NOOP
    void readIR(std::string &)
    {
    } // NOOP
    void writeIR(const std::string &)
    {
    } // NOOP
    CPH5StrIOFacility *getIOFacility()
    {
        return 0;
    }
    H5::Attribute *createAttribute(std::string /*name*/,
                                   H5::DataType /*dataType*/,
                                   H5::DataSpace /*space*/)
    {
        return 0;
    }
    H5::Attribute *openAttribute(std::string /*name*/)
    {
        return 0;
    }
    void registerAttribute(CPH5AttributeInterface *)
    {
    } // NOOP
    void unregisterAttribute(const CPH5AttributeInterface *)
    {
    } // NOOP
    void extendIR(int, int)
    {
    } // NOOP
    int getDimSizeIR(int)
    {
        return 0;
    } // NOOP
    int getMaxDimSizeIR(int)
    {
        return 0;
    } // NOOP

    // These may or may not be necessary.
    CPH5TreeNode::CPH5LeafType getLeafType() const
    {
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    bool getValIfLeaf(void * /*p*/)
    {
        return false;
    }
    bool canIndexInto() const
    {
        return false;
    }
    CPH5TreeNode *indexInto(int /*i*/)
    {
        return 0;
    }
    int getIndexableSize() const
    {
        return 0;
    }

    void *getMemoryLocation() const { return 0; }

    std::vector<std::string> getChildrenNames() const
    {
        return std::vector<std::string>();
    }
    CPH5TreeNode *getChildByName(std::string /*name*/) const
                                 {
        return 0;
    }

private:
    CPH5VarLenStr()
    :
            CPH5VarLenStrBase<CPH_5_MAX_DIMS + 1>(0)
    {
    } // NOOP
    CPH5VarLenStr(const CPH5VarLenStr &other);
};
/*!
 * \endcond
 */

#endif // CPH5VARLENSTR_H
