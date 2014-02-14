<?php

namespace Apigee\Util;

/**
 * The base class for a cache manager. 
 * Cache classes must extend this class and provide a default constructor.
 *
 * <p>A Factory class creates an instance of the class and 
 * then invokes the setup() method.
 * The setup() method is passed an argument that is returned
 * by the getConfig() method, which means setup logic can be performed
 * outside of the constructor. </p>
 *
 * <p>Note that the setup() method should be
 * invoked once and only once by the Factory class.</p>
 *
 * @author isaias
 *
 */
class CacheManager {

  private $config;

  /**
   * @internal
   */
  public function __construct() {
    $GLOBALS['ApigeeMintCache'] = array();
  }

  /**
   * Since static methods are not overridden, the Factory class
   * invokes this method right after the cache manager is created. 
   * That means setup logic can be executed here instead of in the constructor.
   *
   * @param array $config
   */
  public function setup($config) {
    $this->config = $config;
  }

  /**
   * Returns a config array that is later
   * passed to the setup() method by the Factory class.
   *
   * @return array
   */
  public function getConfig() {
    return array();
  }

  /**
   * Cache a value given $data and identifying it by $cid
   *
   * @param string $cid
   * @param mixed $data
   */
  public function set($cid, $data) {
    $GLOBALS['ApigeeMintCache'][$cid] = $data;
  }

  /**
   * Attempt to get a value from cache given the ID specified by $cid.
   * If no value is found in the cache, then value specified by $data is
   * returned. 
   * If no $data is specified, return NULL.
   *
   * @param string $cid
   * @param mixed $data
   */
  public function get($cid, $data = NULL) {
    if (array_key_exists($cid, $GLOBALS['ApigeeMintCache'])) {
      $data = $GLOBALS['ApigeeMintCache'][$cid];
    }
    return $data;
  }

  public function clear($cid = NULL, $wildcard = FALSE) {
    if (empty($cid) && $wildcard === TRUE) {
      $GLOBALS['ApigeeMintCache'] = array();
    }
    elseif ($wildcard === TRUE) {
      foreach ($GLOBALS['ApigeeMintCache'] as $cache_id) {
        if (strpos($cache_id, $cid) === 0) {
          unset($GLOBALS['ApigeeMintCache'][$cache_id]);
        }
      }
    }
    else {
      unset($GLOBALS['ApigeeMintCache'][$cid]);
    }
  }
}