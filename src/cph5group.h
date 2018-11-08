////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

#ifndef CPH5GROUP_H
#define CPH5GROUP_H

#include <vector>
#include <string>
#include <iostream>

#include "cph5utilities.h"


#ifdef _MSC_VER
#pragma warning(disable: 4355 4706 4115 4100 4201 4214 4054)
#endif /* _MSC_VER */



/*!
 * \brief The CPH5Group class is the container representing an
 *        HDF5 Group object to be subclassed with implementation
 *        specific members.
 * 
 * This class simultaneously describes HDF5 Groups, and the parent
 * HDF5 file. Groups can belong to other Groups, and the root of
 * every HDF5 file is a group with the name "/". CPH5Groups use
 * recursion in order to open/create and close the file described
 * by the root group. They also provide interface functions necessary
 * for children to access the H5 objects that they need.
 * 
 * In order to use this class, users should subclass it into a struct
 * and implemented members subclassed from other CPH5Group,
 * CPH5Dataset, or CPH5Attribute classes.
 */
class CPH5Group : public CPH5GroupMember, public CPH5AttributeHolder
{
public:
    
    /*!
     * \brief CPH5Group Constructor to be used if the group is to be a child
     *        of another group.
     * \param parent The parent group for this group.
     * \param name The name of this group visible in the target HDF5 file.
     */
    CPH5Group(CPH5Group *parent, std::string name)
        : CPH5GroupMember(name),
          mpParent(parent),
          mpFile(0),
          mpGroup(0)
    {
        if (mpParent != 0)
            mpParent->registerChild(this);
    }
    
    
    /*!
     * \brief CPH5Group Constructor to be used for a root-level group, i.e.
     *        not a child of another group. This constructor should be used
     *        at the file level.
     */
    CPH5Group()
        : CPH5GroupMember("/"),
          mpParent(0),
          mpFile(0),
          mpGroup(0)
    {
        //NOOP
    }
    
    
    /*!
     * CPH5Group Destructor. Recursively closes all children and the file 
     *           pointer object if one has been created.
     */
    ~CPH5Group()
    {
        closeR();
        
        // Delete all the external children
        for (int i = 0; i < mExternalChildren.size(); ++i) {
            delete mExternalChildren[i];
        }
        mExternalChildren.clear();
        mAdopteeChildren.clear();
        if (mpFile != 0) {
            mpFile->close();
            delete mpFile;
            mpFile = 0;
        }
    }
    
    
    /*!
     * \brief Function to create or overwrite the target HDF5 file with the
     *        given name (path should be included if it is not a relative
     *        name). The first thing this function does is call
     *        createOrOverwriteAssist with the filename. If that function 
     *        returns true it continues, otherwise it will not continue to
     *        make the target HDF5 file. After that it creates the file and
     *        H5::Group objects and recursively opens the children. When it is
     *        done if there have not been any problems it will call the 
     *        function createOrOverwriteComplete to signal that it was
     *        successful. This function will not execute unless this group
     *        object has no parent.
     * \param filename Name of target HDF5 file.
     * \return True if success, false if otherwise.
     */
    bool createOrOverwriteFile(std::string filename) {
        if (!mFileName.empty()) {
            return false; // Group is already open
        }
        if (!createOrOverwriteAssist(filename)) {
            return false;
        }
        // CANNOT DO THIS FOR NON-ROOT GROUP
        if (mpParent == 0) {
            mpFile = new H5::H5File(filename.c_str(), H5F_ACC_TRUNC);
            mpGroup = new H5::Group(mpFile->openGroup(mName));
            for (ChildList::iterator it = mChildren.begin();
                 it != mChildren.end();
                 ++it) {
                (*it)->openR(true);
            }
            mFileName = filename;
            createOrOverwriteComplete();
            return true;
        }
        return false;
    }
    
    
    /*!
     * \brief Opens the target HDF5 file instead of creating it. This function
     *        will not execute unless this group object has no parent. Creates
     *        the file and H5::Group objects and recursively opens the children.
     * \param filename Name of target HDF5 file.
     */
    void openFile(std::string filename, bool readOnly = false) {
        if (!mFileName.empty()) {
            return; // Group is already open
        }
        // CANNOT DO THIS FOR NON-ROOT GROUP
        if (mpParent == 0) {
            if (readOnly) {
                mpFile = new H5::H5File(filename.c_str(), H5F_ACC_RDONLY);
            } else {
                mpFile = new H5::H5File(filename.c_str(), H5F_ACC_RDWR);
            }
            mpGroup = new H5::Group(mpFile->openGroup(mName));
            for (ChildList::iterator it = mChildren.begin();
                 it != mChildren.end();
                 ++it) {
                (*it)->openR(false);
            }
            mFileName = filename;
        }
    }
    
    
    /*!
     * \brief This function 'opens' an HDF5 file in memory - nothing is
     *        written to or read from disk.
     * \param uniqueName Name for file even though it is not written to disk,
     *        must be unique.
     * \param memoryIncrement Size of memory block to allocate whenever
     *        more memory is needed by the file.
     * \return Boolean - successful open
     */
    bool openInMemory(std::string uniqueName,
                      int memoryIncrement = 1048576) { // Default: 1MB
        if (!mFileName.empty()) {
            return false; // Group is already open
        }
        // CANNOT DO THIS FOR NON-ROOT GROUP
        if (mpParent != 0) {
            return false;
        }
        H5::FileAccPropList propList;
        H5Pset_fapl_core(propList.getId(), memoryIncrement, false);
        mpFile = new H5::H5File(uniqueName,
                                H5F_ACC_TRUNC,
                                H5::FileCreatPropList::DEFAULT,
                                propList);
        mpGroup = new H5::Group(mpFile->openGroup(mName));
        for (ChildList::iterator it = mChildren.begin();
             it != mChildren.end();
             ++it) {
            (*it)->openR(true);
        }
        mFileName = uniqueName;
        return true;
    }
    
    
    
    /*!
     * \brief Closes the target HDF5 file and by extension all children objects
     *        recursively. Will not run if this group object has a parent.
     */
    void close() {
        if (mpParent == 0) {
            // CANNOT BE DONE ON NON-ROOT GROUP
            closeR();
            if (mpFile != 0) {
                mpFile->close();
                delete mpFile;
                mpFile = 0;
            }
            mFileName.clear();
            return;
        }
    }
    
    /*!
     * \brief Adopts an HDF5 group and opens it up in the target file if the file
     *        is open
     * \param shared ptr to the group to add
     */
    void adoptAndOpen(std::shared_ptr<CPH5GroupMember> adoptee)
    {
        //add the pointer to the vector
        mAdopteeChildren.push_back(adoptee);

        //check to see if we are open
        if (mpGroup == nullptr)
        {
            //not open yet so only add
            return;
        }

        //file is already open so try and open the group
        adoptee->openR(false);

    }

    /*!
     * \brief Adopts an HDF5 group and creates it in the target file if the file
     *        is open
     * \param shared ptr to the group to add
     */
    void adoptAndCreate(std::shared_ptr<CPH5GroupMember> adoptee)
    {
        //add the pointer to the vector
        mAdopteeChildren.push_back(adoptee);

        //check to see if we are open
        if (mpGroup == nullptr)
        {
            //not open yet so only add
            return;
        }

        //file is already open so try and open the group
        adoptee->openR(true);

    }
    
    /*!
     * \brief Creates an H5::Group in the target H5 file if it has been opened.
     * \param name Name to assign to group visible in target HDF5 file.
     * \return Pointer to newly created group object, or 0 if failure.
     */
    H5::Group *createGroup(std::string name) {
        if (mpGroup != 0)
            return new H5::Group(mpGroup->createGroup(name));
        return 0;
    }
    
    
    /*!
     * \brief Opens an H5::Group in the target H5 file if it has been opened.
     * \param name Name of group to open visible in the target HDF5 file.
     * \return Pointer to newly created H5::Group object, or 0 if failure.
     */
    H5::Group *openGroup(std::string name) {
        if (mpGroup != 0)
            return new H5::Group(mpGroup->openGroup(name));
        return 0;
    }
    
    
    /*!
     * \brief Creates a child dataset for this group object in the target
     *        HDF5 file. 
     * \param name Name to assign to dataset visible in the target HDF5 file.
     * \param dataType Type to make dataset of.
     * \param space Filespace to create dataset with.
     * \param prop Optional property list - used if the chunk size is set.
     * \return Pointer to newly created H5::DataSet object, or 0 if failure.
     */
    H5::DataSet *createDataSet(std::string name,
                               H5::DataType dataType,
                               H5::DataSpace space,
                               H5::DSetCreatPropList prop) {
        if (mpGroup != 0) {
            return new H5::DataSet(mpGroup->createDataSet(name,
                                                          dataType,
                                                          space,
                                                          prop));
        }
        return 0;
    }
    
    
    /*!
     * \brief Overload of other createDataSet function but without the 
     *        property list parameter.
     * \param name Name to assign to dataset visible in the target HDF5 file.
     * \param dataType Type to make dataset of.
     * \param space Filespace to create dataset with.
     * \return Pointer to newly created H5::DataSet object, or 0 if failure.
     */
    H5::DataSet *createDataSet(std::string name,
                               H5::DataType dataType,
                               H5::DataSpace space) {
        if (mpGroup != 0) {
            return new H5::DataSet(mpGroup->createDataSet(name,
                                                          dataType,
                                                          space));
        }
        return 0;
    }
    
    
    /*!
     * \brief Opens a dataset in the target HDF5 file. 
     * \param name Name of dataset visible in the target HDF5 file.
     * \return Pointer to newly created H5::DataSet object, or 0 if failure.
     */
    H5::DataSet *openDataSet(std::string name) {
        if (mpGroup != 0)
            return new H5::DataSet(mpGroup->openDataSet(name));
        return 0;
    }
    
    
    /*!
     * \brief Creates a child attribute for this group in the target HDF5 file
     *        if it is open.
     * \param name Name to assign to attribute visible in the target HDF5 file.
     * \param dataType Type to create attribute with.
     * \param space Filespace to create attribute with.
     * \return Pointer to newly created H5::Attribute object, or 0 if failure.
     */
    H5::Attribute *createAttribute(std::string name,
                                   H5::DataType dataType,
                                   H5::DataSpace space) {
        if (mpGroup != 0)
            return new H5::Attribute(
                        mpGroup->createAttribute(name,
                                                 dataType,
                                                 space,
                                                 H5::PropList::DEFAULT));
        return 0;
    }
    
    
    /*!
     * \brief Opens a child attribute with the given name in the target HDF5
     *        file if it is open.
     * \param name Name of attribute visible in the target HDF5 file.
     * \return Pointer to newly created H5::Attribute object or 0 if failure.
     */
    H5::Attribute *openAttribute(std::string name) {
        if (mpGroup != 0)
            return new H5::Attribute(mpGroup->openAttribute(name));
        return 0;
    }
    
    
    /*!
     * \brief Registers a CPH5Attribute object as a child of this group object.
     * \param child Pointer to CPH5Attribute object to register.
     */
    void registerAttribute(CPH5AttributeInterface *child) {
        registerChild(child);
    }
    
    
    /*!
     * \brief Unregisters a CPH5Attribute object from this groups list of
     *        children.
     * \param child Pointer to CPH5Attribute object to unregister.
     */
    void unregisterAttribute(const CPH5AttributeInterface *child) {
        unregisterChild(child);
    }
    
    
    /*!
     * \brief Registers a CPH5GroupMember (either CPH5Dataset or another
     *        CPH5Group) as a child of this group object.
     * \param child Pointer to object to register as child.
     */
    void registerChild(CPH5GroupMember *child) {
        mChildren.push_back(child);
    }
    
    /*!
     * \brief Same as registerChild, but adds the child to a list of objects
     *        to be deleted when this object is de-structed. When the child
     *        is constructed it should be given the pointer to what its
     *        parent will be, this function won't set that up.
     * \param child Pointer to object to register as child and delete later.
     */
    void registerExternalChild(CPH5GroupMember *child) {
        mExternalChildren.push_back(child);
        //mChildren.push_back(child); Don't do this.
    }

    /*!
     * \brief Unregisters a CPH5GroupMember (either a CPH5Dataset or another
     *        CPH5Group) from this groups list of children.
     * \param child Pointer to object to unregister.
     */
    void unregisterChild(const CPH5GroupMember *child) {
        for(ChildList::iterator it = mChildren.begin();
            it != mChildren.end();
            ++it) {
            if ((*it) == child) {
                it = mChildren.erase(it);
            }
        }
    }
    
    
    /*!
     * \brief Returns the file name of the currently open target HDF5 file.
     *        Will be an empty string if the file is not currently open.
     * \return Name of target HDF5 file if it is open.
     */
    std::string getFilename() const {
        return mFileName;
    }
    
    
    /*!
     * \brief operator = Overloaded assignment operator - this function is a
     *        no-op. It needs to be defined so that calls to assignment from
     *        classes that inherit group can use assignment on their members,
     *        but we don't want any parameters maintained by this base class
     *        to be copied.
     * \param other
     */
    void operator=(const CPH5Group &other) {} // NOOP
    
    
    /*!
     * \brief numChildren Returns number of children in this group
     * \return Number of children in the group.
     */
    int numChildren() const {
       return mChildren.size();
    }
    
    
    /*!
     * \brief childAt Accessor function for children of this group
     * \param i Index of child in list. Must be 0 <= i < num children
     * \return Pointer to child
     */
    CPH5GroupMember *childAt(int i) const {
       if (i < 0 || i >= mChildren.size()) return 0;
       return mChildren.at(i);
    }
    
    /*!
     * \brief getH5File Accessor function for the H5File object that
     *        this group maintains.
     * \return Pointer to H5File object maintained by this group.
     */
    H5::H5File *getH5File() const {
        return mpFile;
    }
    
    /*!
     * \brief getH5Group Accessor function for the H5::Group object that
     *        this group maintains.
     * \return Pointer to H5::Group object maintained by this group.
     */
    H5::Group *getH5Group() const {
        return mpGroup;
    }
    
    //TODO document
    CPH5LeafType getLeafType() const override {
        // A group is never a leaf.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    bool getValIfLeaf(void *p) override {
        // A group is never a leaf.
        return false;
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
        // Groups do not have an element type.
        return CPH5TreeNode::LT_IS_NOT_LEAF;
    }
    
    //TODO document
    int getMemorySizeBelow() const {
        // Cannot use this at a group level.
        return 0;
    }
    
    //TODO document
    bool readAllBelow(void *p) {
        // Cannot use this at a group level.
        return false;
    }
    
    //TODO document
    void *getMemoryLocation() const {
        return 0;
    }
    
    //TODO document
    std::vector<std::string> getChildrenNames() const override {
        std::vector<std::string> ret;
        if (!mChildren.empty()) {
            for (int i = 0; i < mChildren.size(); ++i) {
                ret.push_back(mChildren.at(i)->getName());
            }
        }
        return ret;
    }
    
    //TODO document
    CPH5TreeNode *getChildByName(std::string name) const override {
        if (!mChildren.empty()) {
            for (int i = 0; i < mChildren.size(); ++i) {
                if (mChildren.at(i)->getName() == name) {
                    return dynamic_cast<CPH5TreeNode*>(mChildren.at(i));
                }
            }
        }
        return 0;
    }
    
    
protected:
    
    // Explicitly disable copy constructor and assignment operator
    CPH5Group(const CPH5Group &other);
    
    using SharedChildList = std::vector<std::shared_ptr<CPH5GroupMember>>;

    typedef std::vector<CPH5GroupMember*> ChildList;
    ChildList mChildren;
    
    // All pointers in this list will also be in mChildren.
    ChildList mExternalChildren;
    
    // All pointers in here are for adoptee groups only
    SharedChildList mAdopteeChildren;
    
    /*!
     * \brief Recursive open function. Creates or opens H5::Group object and 
     *        recursively opens all children.
     * \param create Flag for whether to create or open the group in the target
     *        HDF5 file.
     */
    void openR(bool create) {
        // This should never be called on the root group.
        if (mpParent == 0)
            return;
        
        if (create)
            mpGroup = new H5::Group(mpParent->mpGroup->createGroup(mName));
        else
            mpGroup = new H5::Group(mpParent->mpGroup->openGroup(mName));
        
        for (ChildList::iterator it = mChildren.begin();
             it != mChildren.end();
             ++it) {
            (*it)->openR(create);
        }

        for (SharedChildList::iterator it = mAdopteeChildren.begin();
                it != mAdopteeChildren.end();
                ++it) {
            (*it)->openR(create);
        }
        //mpGroup->close();
    }
    
    
    /*!
     * \brief Recursive close function. Recursively closes all children and
     *        then deletes the H5::Group object if it exists.
     */
    void closeR() {
        if (!mChildren.empty()) {
            for (ChildList::iterator it = mChildren.begin();
                 it != mChildren.end();
                 ++it) {
                (*it)->closeR();
            }
        }
        if (!mAdopteeChildren.empty()) {
            for (SharedChildList::iterator it = mAdopteeChildren.begin();
                    it != mAdopteeChildren.end();
                    ++it) {
                (*it)->closeR();
            }
        }
        if (mpGroup) {
            mpGroup->close();
            delete mpGroup;
            mpGroup = 0;
        }
    }
    
    
    CPH5Group *mpParent;
    H5::Group *mpGroup;
    H5::H5File *mpFile;
    
private:
    
    
    /*!
     * \brief This function is called at the beginning of createOrOverwriteFile
     *        and can be overridden by the user to allow for some additional
     *        checking for whether the file can be created (or overwritten if
     *        it exists).
     * \param filename Name of target HDF5 file.
     * \return Defaults to true, user should return false if the file should
     *         not be overwritten (such as if the user were to hit a cancel
     *         button).
     */
    virtual bool createOrOverwriteAssist(std::string filename) {
        return true;
    }
    
    // Is called at the end of createOrOverwriteFile only if the creation
    // is successful. Can be overridden by subclasses to do some post-
    // opening processing.
    /*!
     * \brief Is called at the end of createOrOverwriteFile if it has been
     *        successful. Can be overridden by the user to do some post
     *        processing.
     */
    virtual void createOrOverwriteComplete() {
        // NOOP
    }
    
    std::string mFileName;
    
};


/*!
 * \brief The CPH5EmptyGroup struct is a convenience class to define
 *        an empty group.
 * 
 * Since the CPH5 library does not have to describe the entire .h5
 * file, empty groups can be used to designate that there IS a group
 * present but either without members or with only members that we
 * are not concerned with.
 */
struct CPH5EmptyGroup : public CPH5Group
{
    /*!
     * \brief Default constructor. See CPH5Group constructor.
     */
    CPH5EmptyGroup()
        : CPH5Group()
    {
        //NOOP
    }
    
    /*!
     * \brief Default constructor, pass through. See CPH5Group constructor.
     * \param parent Parent group object.
     * \param name Name of this group
     */
    CPH5EmptyGroup(CPH5Group *parent, std::string name)
        : CPH5Group(parent, name)
    {
        //NOOP
    }
};




#endif // CPH5GROUP_H

