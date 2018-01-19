-- Copyright 2018 Stanford University
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

import "regent"
import "bishop"

mapper

task#f {
  target : processors[isa=x86][0];    
}

end

local cextern_task
do
  assert(os.getenv('LG_RT_DIR') ~= nil, "$LG_RT_DIR should be set!")
  local root_dir = arg[0]:match(".*/") or "./"
  local runtime_dir = os.getenv('LG_RT_DIR') .. "/"
  local legion_dir = runtime_dir .. "legion/"
  local mapper_dir = runtime_dir .. "mappers/"
  local realm_dir = runtime_dir .. "realm/"
  local legion_interop_cc = root_dir .. "extern_task.cc"
  local legion_interop_so
  if os.getenv('SAVEOBJ') == '1' then
    legion_interop_so = root_dir .. "libextern_task.so"
  else
    legion_interop_so = os.tmpname() .. ".so" -- root_dir .. "mapper.so"
  end
  local cxx = os.getenv('CXX') or 'c++'

  local cxx_flags = os.getenv('CC_FLAGS') or ''
  cxx_flags = cxx_flags .. " -O2 -Wall -Werror"
  if os.execute('test "$(uname)" = Darwin') == 0 then
    cxx_flags =
      (cxx_flags ..
         " -dynamiclib -single_module -undefined dynamic_lookup -fPIC")
  else
    cxx_flags = cxx_flags .. " -shared -fPIC"
  end

  local cmd = (cxx .. " " .. cxx_flags .. " -I " .. runtime_dir .. " " ..
                 " -I " .. mapper_dir .. " " .. " -I " .. legion_dir .. " " ..
                 " -I " .. realm_dir .. " " .. legion_interop_cc .. " -o " .. legion_interop_so)
  if os.execute(cmd) ~= 0 then
    print("Error: failed to compile " .. legion_interop_cc)
    assert(false)
  end
  terralib.linklibrary(legion_interop_so)
  cextern_task =
    terralib.includec("extern_task.h", {"-I", root_dir, "-I", runtime_dir,
                                           "-I", mapper_dir, "-I", legion_dir,
                                           "-I", realm_dir})
end

struct s {
  a : int32,
  b : int32,
  c : int32,
  d : int32,
}

extern task f(r : region(s), x : regentlib.future(float))
where
  -- Note: With the manual calling convention, these will exactly
  -- correspond to the region requirements passed to the task---so no
  -- grouping or collation of privileges.
  reads(r.{a, b}),
  reads(r.c),
  reads writes(r.d)
end
f:set_task_id(cextern_task.TID_F)
f:set_calling_convention(regentlib.convention.manual())

task main()
  var r = region(ispace(ptr, 5), s)
  fill(r.{a, b}, 1.0)

  f(r, 2.0)
end

terra register_all()
  cextern_task.register_tasks()
  [bishoplib.make_entry()]
end

regentlib.start(main, register_all)
