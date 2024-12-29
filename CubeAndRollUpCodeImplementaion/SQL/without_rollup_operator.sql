EXPLAIN ANALYZE
SELECT
    *
FROM
    (
        -- Level 1: Detailed level (Region, Product, Category, Store, Quarter)
        SELECT
            Region,
            Product,
            Category,
            Store,
            Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
        GROUP BY
            Region,
            Product,
            Category,
            Store,
            Quarter
        UNION ALL
        -- Level 2: Grouped by Region, Product, Category, Store
        SELECT
            Region,
            Product,
            Category,
            Store,
            NULL AS Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
        GROUP BY
            Region,
            Product,
            Category,
            Store
        UNION ALL
        -- Level 3: Grouped by Region, Product, Category
        SELECT
            Region,
            Product,
            Category,
            NULL AS Store,
            NULL AS Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
        GROUP BY
            Region,
            Product,
            Category
        UNION ALL
        -- Level 4: Grouped by Region, Product
        SELECT
            Region,
            Product,
            NULL AS Category,
            NULL AS Store,
            NULL AS Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
        GROUP BY
            Region,
            Product
        UNION ALL
        -- Level 5: Grouped by Region
        SELECT
            Region,
            NULL AS Product,
            NULL AS Category,
            NULL AS Store,
            NULL AS Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
        GROUP BY
            Region
        UNION ALL
        -- Level 6: Total (Grand Total)
        SELECT
            NULL AS Region,
            NULL AS Product,
            NULL AS Category,
            NULL AS Store,
            NULL AS Quarter,
            SUM(Revenue) AS SUM_Revenue,
            COUNT(Revenue) AS COUNT_Revenue,
            MIN(Revenue) AS MIN_Revenue,
            MAX(Revenue) AS MAX_Revenue,
            AVG(Revenue) AS AVG_Revenue
        FROM
            SalesTable
    ) AS aggregated_data
ORDER BY
    Region,
    Product,
    Category,
    Store,
    Quarter;