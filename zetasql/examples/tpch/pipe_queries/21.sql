#
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# EXISTS subqueries omit the placeholder SELECTs.
FROM
  supplier,
  lineitem l1,
  orders,
  nation
|> WHERE
     s_suppkey = l1.l_suppkey
     AND o_orderkey = l1.l_orderkey
     AND o_orderstatus = 'F'
     AND l1.l_receiptdate > l1.l_commitdate
     AND EXISTS(
       FROM lineitem l2
       |> WHERE
            l2.l_orderkey = l1.l_orderkey
            AND l2.l_suppkey <> l1.l_suppkey)
     AND NOT EXISTS(
       FROM lineitem l3
       |> WHERE
            l3.l_orderkey = l1.l_orderkey
            AND l3.l_suppkey <> l1.l_suppkey
            AND l3.l_receiptdate > l3.l_commitdate)
     AND s_nationkey = n_nationkey
     AND n_name = 'PERU'
|> AGGREGATE COUNT(*) AS numwait
   GROUP BY s_name
|> ORDER BY
     numwait DESC,
     s_name
|> LIMIT 100;
