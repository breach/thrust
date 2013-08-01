// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

var apiDispatcher = apiDispatcher || {};

(function() {
  native function RequireBreach();

  apiDispatcher.requireBreach = RequireBreach;
  console.log('INSTALLED');
})();
