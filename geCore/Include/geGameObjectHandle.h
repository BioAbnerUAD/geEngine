/*****************************************************************************/
/**
 * @file    geGameObjectHandle.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/11/23
 * @brief   A handle that can point to various types of game objects.
 *
 * A handle that can point to various types of game objects. It primarily keeps
 * track if the object is still alive, so anything still referencing it doesn't
 * accidentally use it.
 *
 * @note This class exists because references between game objects should be
 *       quite loose. For example one game object should be able to reference
 *       another one without the other one knowing. But if that is the case I
 *       also need to handle the case when the other object we're referencing
 *       has been deleted, and that is the main purpose of this class.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/

namespace geEngineSDK {
  class GameObjectManager;

  template<typename T>
  class GameObjectHandle;

  /**
   * @brief Contains instance data that is held by all GameObject handles.
   */
  struct GameObjectInstanceData
  {
    GameObjectInstanceData() : object(nullptr), instanceId(0) {}
    SPtr<GameObject> object;
    uint64 instanceId;
  };

  typedef SPtr<GameObjectInstanceData> GameObjectInstanceDataPtr;

  /**
   * @brief Internal data shared between GameObject handles.
   */
  struct GE_CORE_EXPORT GameObjectHandleData
  {
    GameObjectHandleData() : m_ptr(nullptr) {}

    GameObjectHandleData(const SPtr<GameObjectInstanceData>& ptr) {
      m_ptr = ptr;
    }

    SPtr<GameObjectInstanceData> m_ptr;
  };

  /**
   * @brief A handle that can point to various types of game objects. It
   *        primarily keeps track if the object is still alive, so anything
   *        still referencing it doesn't accidentally use it.
   * @note  This class exists because references between game objects should be
   *        quite loose. For example one game object should be able to
   *        reference another one without the other one knowing. But if that is
   *        the case I also need to handle the case when the other object we're
   *        referencing has been deleted, and that is the main purpose of this
   *        class.
   */
  class GE_CORE_EXPORT GameObjectHandleBase : public IReflectable
  {
   public:
    GameObjectHandleBase();

    /**
     * @brief Returns true if the object the handle is pointing to has been
     *        destroyed.
     * @param[in] checkQueued Game objects can be queued for destruction but
     *            not actually destroyed yet, and still accessible. If this is
     *            false this method will return true only if the object is
     *            completely inaccessible (fully destroyed). If this is true
     *            this method will return true if object is completely
     *            inaccessible or if it is just queued for destruction.
     */
    bool
    isDestroyed(bool checkQueued = false) const;

    /**
     * @brief Returns the instance ID of the object the handle is referencing.
     */
    uint64
    getInstanceId() const {
      return nullptr != m_data->m_ptr ? m_data->m_ptr->instanceId : 0;
    }

    /**
     * @brief Returns pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    GameObject*
    get() const {
      throwIfDestroyed();
      return m_data->m_ptr->object.get();
    }

    /**
     * @brief Returns a smart pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    SPtr<GameObject>
    getInternalPtr() const {
      throwIfDestroyed();
      return m_data->m_ptr->object;
    }

    /**
     * @brief Returns pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    GameObject*
    operator->() const {
      return get();
    }

    /**
     * @brief Returns reference to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    GameObject&
    operator*() const {
      return *get();
    }

   public:
    /**
     * @brief Returns internal handle data.
     */
    SPtr<GameObjectHandleData>
    _getHandleData() const {
      return m_data;
    }

    /**
     * @brief Resolves a handle to a proper GameObject in case it was created
     *        uninitialized.
     */
    void
    _resolve(const GameObjectHandleBase& object);

    /**
     * @brief Changes the GameObject instance the handle is pointing to.
     */
    void
    _setHandleData(const SPtr<GameObject>& object);

   protected:
    friend class GameObjectManager;

    template<class _Ty1, class _Ty2>
    friend bool
    operator==(const GameObjectHandle<_Ty1>& _Left,
               const GameObjectHandle<_Ty2>& _Right);

    GameObjectHandleBase(const SPtr<GameObject> ptr);
    GameObjectHandleBase(const SPtr<GameObjectHandleData>& data);
    GameObjectHandleBase(std::nullptr_t ptr);

    /**
     * @brief Throws an exception if the referenced GameObject has been
     *        destroyed.
     */
    void
    throwIfDestroyed() const;

    /**
     * @brief Invalidates the handle signifying the referenced object was
     *        destroyed.
     */
    void
    destroy() {
      //It's important not to clear m_data->m_ptr as some code might rely on it
      //(for example for restoring lost handles)

      if (nullptr != m_data->m_ptr) {
        m_data->m_ptr->object = nullptr;
      }
    }

    SPtr<GameObjectHandleData> m_data;

    /*************************************************************************/
    /**
     * RTTI
     */
    /*************************************************************************/
   public:
    friend class GameObjectHandleRTTI;
    
    static RTTITypeBase*
    getRTTIStatic();

    virtual RTTITypeBase*
    getRTTI() const override;
  };

  /**
   * @copydoc GameObjectHandleBase
   * @note  It is important this class contains no data since we often value
   *        cast it to its base.
   */
  template<typename T>
  class GameObjectHandle : public GameObjectHandleBase
  {
   public:
    /**
     * @brief Constructs a new empty handle.
     */
    GameObjectHandle() : GameObjectHandleBase() {
      m_data = ge_shared_ptr_new<GameObjectHandleData>();
    }

    /**
     * @brief Copy constructor from another handle of the same type.
     */
    template<typename T1>
    GameObjectHandle(const GameObjectHandle<T1>& ptr)
      : GameObjectHandleBase() {
      m_data = ptr._getHandleData();
    }

    /**
     * @brief Copy constructor from another handle of the base type.
     */
    GameObjectHandle(const GameObjectHandleBase& ptr)
      : GameObjectHandleBase() {
      m_data = ptr._getHandleData();
    }

    /**
     * @brief Invalidates the handle.
     */
    GameObjectHandle<T>&
    operator=(std::nullptr_t ptr) {
      m_data = ge_shared_ptr_new<GameObjectHandleData>();
      return *this;
    }

    /**
     * @brief Returns a pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    T*
    get() const {
      throwIfDestroyed();
      return reinterpret_cast<T*>(m_data->m_ptr->object.get());
    }

    /**
     * @brief Returns a smart pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    SPtr<T>
    getInternalPtr() const {
      throwIfDestroyed();
      return std::static_pointer_cast<T>(m_data->m_ptr->object);
    }

    /**
     * @brief Returns pointer to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    T*
    operator->() const {
      return get();
    }

    /**
     * @brief Returns reference to the referenced GameObject.
     * @note  Throws exception if the GameObject was destroyed.
     */
    T&
    operator*() const {
      return *get();
    }

   public:
    template<class _Ty>
    struct Bool_struct
    {
      int _member;
    };

    /**
     * @brief Allows direct conversion of handle to bool.
     * @note This is needed because we can't directly convert to bool since
     *        then we can assign pointer to bool and that's weird.
     */
    operator int
    Bool_struct<T>::*() const {
      return (((nullptr != m_data->m_ptr) && (nullptr != m_data->m_ptr->object))
        ? &Bool_struct<T>::_member : 0);
    }
  };

  /**
   * @brief Casts one GameObject handle type to another.
   */
  template<class _Ty1, class _Ty2>
  GameObjectHandle<_Ty1>
  static_object_cast(const GameObjectHandle<_Ty2>& other) {
    return GameObjectHandle<_Ty1>(other);
  }

  /**
   * @brief Compares if two handles point to the same GameObject.
   */
  template<class _Ty1, class _Ty2>
  bool
  operator==(const GameObjectHandle<_Ty1>& _Left,
             const GameObjectHandle<_Ty2>& _Right) {
    return (nullptr == _Left.m_data && nullptr == _Right.m_data) ||
           (nullptr != _Left.m_data && nullptr != _Right.m_data  &&
           _Left.getInstanceId() == _Right.getInstanceId());
  }

  /**
   * @brief Compares if two handles point to different GameObject%s.
   */
  template<class _Ty1, class _Ty2>
  bool
  operator!=(const GameObjectHandle<_Ty1>& _Left,
             const GameObjectHandle<_Ty2>& _Right) {
    return (!(_Left == _Right));
  }
}
